<?php

/**
 * Retrieve Webform components of Node Modal settings.
 *
 * @param int $nid
 *   The node nid.
 * @return array
 *  Webform components.
 */
function node_modal_get_webform_components($nid) {
  $webform_components = db_select('node_modal', 'n')
    ->fields('n', array('webform_components'))
    ->condition('n.nid', $nid)
    ->execute()
  ->fetchField();

  $result = !empty($webform_components) ? unserialize($webform_components) : '';
  return $result;
}

/**
 * Retrieve Webform list.
 *
 * @return array
 *  Webform options.
 */
function node_modal_get_webform_list() {
  // Get list of webforms.
  $nodes = node_load_multiple(array(), array(
    'type' => 'webform',
  ));

  $options = array();
  foreach ($nodes as $nid => $node) {
    $options[$nid] = check_plain($node->title);
  }

  return $options;
}

/**
 * Retrieve Node list.
 *
 * @return array
 *  Node options.
 */
function node_modal_get_node_list() {
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', array('flow_path', 'offer_wall'), 'IN')
    ->fieldCondition('field_available_as_modal', 'value', 1);
  $result = $query->execute();

  if (isset($result['node'])) {
    $node_ids = array_keys($result['node']);
    $nodes = node_load_multiple($node_ids);
  }

  $options = array();
  if (!empty($nodes)) {
    foreach ($nodes as $node_id => $node) {
      $options[$node_id] = check_plain($node->title);
    }
  }

  return $options;
}

/**
 * Check if Clientside validation is enabled for current form.
 *
 * @return boolean
 */
function node_modal_check_clientside_validation($form_id) {
  if (!module_exists('clientside_validation')) {      
    return FALSE;
  }  
  
  $page_match = TRUE;
  $path_filter = variable_get('clientside_validation_path_list', '');
  $in_or_exclude = variable_get('clientside_validation_filter_paths', CLIENTSIDE_VALIDATION_EXCLUDE_PATHS);
  
  if (!empty($path_filter)) {
    $path = drupal_get_path_alias($_GET['q']);

    // Compare with the internal and path alias (if any).
    $page_match = drupal_match_path($path, $path_filter);

    if ($path != $_GET['q']) {
      $page_match = $page_match || drupal_match_path($_GET['q'], $path_filter);
    }
  
  // When $in_or_exclude has a value of 0 (CLIENTSIDE_VALIDATION_EXCLUDE_PATHS),
    // Clientside Validation is added on all pages except those listed in $path_filter.
    // When set to 1 (CLIENTSIDE_VALIDATION_INCLUDE_PATHS), it is displayed only on
    // those pages listed in $path_filter.
    $page_match = !((bool)$in_or_exclude xor $page_match);
  }

  if (!$page_match) {
    return FALSE;
  }

  $validate_specific_setting = variable_get('clientside_validation_validate_specific', '');
  $validate_specific = preg_split('/(\r\n?|\n)/', $validate_specific_setting, -1, PREG_SPLIT_NO_EMPTY);

  if (variable_get('clientside_validation_validate_all', CLIENTSIDE_VALIDATION_VALIDATE_ALL) == CLIENTSIDE_VALIDATION_VALIDATE_SPECIFIC
    && !in_array($form_id, $validate_specific)) {
    return FALSE;
  }
  elseif (variable_get('clientside_validation_validate_all', CLIENTSIDE_VALIDATION_VALIDATE_ALL) == CLIENTSIDE_VALIDATION_VALIDATE_ALL_EXCEPT
    && in_array($form_id, $validate_specific)) {
    return FALSE;
  }
  
  return TRUE;
}