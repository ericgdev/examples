<?php

/**
 * Replace tokens within a URL, encoding the parts within the query string.
 *
 * @param string $redirect_url
 *   The redirect URL, with everything other than tokens already URL encoded.
 * @param $node
 *   If replacing node-level tokens, the node for which tokens will be created.
 * @param $submission
 *   If replacing submission-level tokens, the submission for which tokens will
 *   be created.
 * @return array
 *   An array of path and url() options, suitable for a redirect or drupal_goto.
 */
function webform_popup_helper_replace_url_tokens($redirect_url, $node = NULL, $submission = NULL) {
  // Parse the url into its components.
  $parsed_redirect_url = drupal_parse_url($redirect_url);
  // Replace tokens in each component.
  $parsed_redirect_url['path'] = webform_popup_helper_replace_tokens($parsed_redirect_url['path'], $node, $submission);
  
  if (!empty($parsed_redirect_url['query'])) {
    foreach ($parsed_redirect_url['query'] as $key => $value) {
      $parsed_redirect_url['query'][$key] = trim(webform_popup_helper_replace_tokens($value, $node, $submission));
    }
  }
  
  $parsed_redirect_url['fragment'] = webform_popup_helper_replace_tokens($parsed_redirect_url['fragment'], $node, $submission);
  // Determine whether the path is internal or external. Paths which contain the site's
  // base url are still considered internal. #webform_external is private to webform.
  $parsed_redirect_url['#webform_external'] = url_is_external($parsed_redirect_url['path']);
  
  foreach (array(NULL, TRUE, FALSE) as $https) {
    if (stripos($parsed_redirect_url['path'], url('', array('absolute' => TRUE, 'https' => $https))) === 0) {
      $parsed_redirect_url['#webform_external'] = FALSE;
    }
  }
  
  // Return an array suitable for a form redirect or drupal_goto.
  return array($parsed_redirect_url['path'], $parsed_redirect_url);
}

/*
 * Replace tokens with Webform contexts populated.
 *
 * @param $string
 *   The string to have its tokens replaced.
 * @param $node
 *   If replacing node-level tokens, the node for which tokens will be created.
 * @param $submission
 *   If replacing submission-level tokens, the submission for which tokens will
 *   be created.
 * @param $email
 *   If replacing tokens within the context of an e-mail, the Webform e-mail
 *   settings array.
 * @param $sanitize
 *   Boolean or format name value indicating if the results will be displayed as
 *   HTML output. If FALSE, the contents returned will be unsanitized. This will
 *   also result in all Webform submission tokens being returned as plain-text,
 *   without HTML markup, in preparation for e-mailing or other text-only
 *   purposes (default values, etc.). If TRUE, the tokens only are sanitized by
 *   token_replace. Otherwise $sanitize is the machine name of an import filter
 *   to be used with check_markup().
 */
function webform_popup_helper_replace_tokens($string, $node = NULL, $submission = NULL, $email = NULL, $sanitize = FALSE) {
  // Don't do any filtering if the string is empty.
  if (!strlen(trim($string)) || !webform_variable_get('webform_token_access')) {
    return $string;
  }

  $token_data = array();
  if ($node) {
    $token_data['node'] = $node;
  }
  if ($submission) {
    $token_data['webform-submission'] = $submission;
  }
  if ($email) {
    $token_data['webform-email'] = $email;
  }
  $clear = is_bool($sanitize);
  $string = token_replace($string, $token_data);
  if (!$clear) {
    $string = webform_replace_tokens_clear(check_markup($string, $sanitize));
  }
  return $string;
}

/**
 * Get Webform popup redirect urls.
 *
 * @param object $node
 *   The node object.
 *
 * @param string $type
 *   The redirect urls type: defaults, duplicate or conditionals.
 *
 * @return array
 *   An array of urls.
 */
function webform_popup_helper_get_redirect_urls($node, $type = 'defaults') {
  switch($type) {
    case 'defaults':
      $urls = isset($node->webform['window_urls']) && !empty($node->webform['window_urls']) ? unserialize($node->webform['window_urls']) : array();
      array_shift($urls);
    break;

    case 'duplicate':
      $urls = isset($node->webform['window_duplicate_urls']) && !empty($node->webform['window_duplicate_urls']) ? unserialize($node->webform['window_duplicate_urls']) : array();
      array_shift($urls);
    break;
  

    case 'conditionals':
      $urls = isset($node->webform['window_cond_urls']) && !empty($node->webform['window_cond_urls']) ? unserialize($node->webform['window_cond_urls']) : array();    
    break;
  }
  
  if (!empty($urls)) {
    foreach ($urls as $key => $url) {
      $path = webform_popup_helper_replace_url_tokens($url['url'], $node);
      $query = (!empty($path[1]['query'])) ? $path[1]['query'] : '';
      $path = url($path[1]['path'], array('query' => $query));
      $urls[$key]['url'] = $path;
    }
  }

  return $urls;
}

/**
 * Get Webform server redirect url.
 *
 * @param object $node
 *   The node object.
 * @param $submission
 *   The webform submisssion object.
 * @return array
 *   An array of urls.
 */
function webform_popup_helper_get_server_redirect_url($node = NULL, $submission = NULL) {
  $window_cond_urls = isset($node->webform['window_cond_urls']) ? unserialize($node->webform['window_cond_urls']) : array();
  $window_duplicate_urls = isset($node->webform['window_duplicate_urls']) ? unserialize($node->webform['window_duplicate_urls']) : array();
  $window_urls = isset($node->webform['window_urls']) ? unserialize($node->webform['window_urls']) : array();

  $use_conditionals = FALSE;
  
  // Loop through the conditional urls.
  if (!empty($window_cond_urls)) {
    foreach ($window_cond_urls as $wcu_id => $window_cond_url) {
      // Matches must match number of conditionals in order to pass.
      $matches = 0;
      $data = $submission->data[$window_cond_url['component']];

      // Loop through each data in case field is multivalued.
      foreach ($data as $input_value) {
        if (_webform_popup_helper_conditional_value_match($input_value, $window_cond_url)) {
          $matches++;
          break;
        }
      }

      // If we don't have a match, remove it from the array.
      if ($matches != TRUE) {
        unset($window_cond_urls[$wcu_id]);
      }
    }
    
    if (!empty($window_cond_urls)) {
      $redirect_url = reset($window_cond_urls);
      $use_conditionals = TRUE;    
    }
  }
  
  if ($use_conditionals == FALSE) {
    if (!empty($window_duplicate_urls)) {
      if ($cid = webform_get_cid($node, 'email_address')) {
        // Check if email already exists in database.	  
        $email = $submission->data[$cid[0]];
        $email = (!empty($email)) ? $email[0] : '';
        $email_exists = webform_popup_helper_validate_unique($node->nid, $cid[0], $email, $submission->sid);
      
        // Redirect to duplicate url.
        if ($email_exists == TRUE) {
          $redirect_url = reset($window_duplicate_urls);
        }        
        else if (!empty($window_urls)) { // Redirect to default url.
          $redirect_url = reset($window_urls);
        }
      }
    } 
  } 

  return !empty($redirect_url) ? webform_replace_url_tokens($redirect_url['url'], $node, $submission) : FALSE;
}

/**
 * Test a webform field value against a conditional.
 *
 * @param string $input_value
 *   Webform field input value.
 * @param array $conditional
 *   Conditional values array.
 *
 * @return bool
 *   Matched.
 */
function _webform_popup_helper_conditional_value_match($input_value, $conditional) {
  $matches = array_map('trim', explode(',', $conditional['values']));
  foreach ($matches as $match) {
    if ($input_value == $match) {
      return TRUE;
    }
  }

  return FALSE;
}

/**
 * Callback function for array_filter to remove empty values (not includes zeros).
 */
function webform_popup_helper_filter_empty_arrays($array) {
  $filtered_array = array_map('trim', $array);
  $filtered_array = array_filter($filtered_array, 'strlen');
  
  return count($array) == count($filtered_array);
}

/**
 * Validate a form component value is unique with no duplicates in the database.
 *
 * @param int $nid
 *   The node id.
 *
 * @param int $cid
 *   The webform component id.
 *
 * @param string $value
 *   The webform component value.
 * 
 * @return boolean
 */
function webform_popup_helper_validate_unique($nid, $cid, $value, $sid) {
  if (!$nid || !$cid || !$value) {
    return FALSE;
  }

  $query = db_select('webform_submitted_data')
    ->fields('webform_submitted_data', array('sid'))
    ->condition('nid', $nid)
    ->condition('cid', $cid)
    ->condition('data', $value)
    // More efficient than using countQuery() for data checks.
    ->range(0, 1);
   
  if ($sid) {
    $query->condition('sid', $sid, '<>');
  }  

  return (bool) $query->execute()->fetchField();
}