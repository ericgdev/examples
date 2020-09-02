(function($) {
  Drupal.behaviors.webform_popup_helper = {
    attach: function (context, settings) {
      // Check if a variable is an Array.
      function isArray(myArray) {
        return myArray.constructor.toString().indexOf('Array') > -1;
      }
	  
      // Get form node id.
      function webformPopupHelperGetNodeId(form) {
        var form_classes = form.attr('class').split(' ');
        var form_nid_class = form_classes[1];
        form_classes = form_nid_class.split('webform-client-form-');

        return form_classes[1];
      }
	  

      // Get form element value.
      function webformPopupHelperGetElementValue(form, elementName) {
        var elementValues = [];
        form.find('[name^=submitted\\[' + elementName + '\\]]').each(
          function(index, field){
            var element = $(this);
            if (element.is(':checkbox') || element.is(':radio')) {
              if (element.is(':checked')) {
                elementValues.push(element.val());
              }
            } else if (element.is('select')) {
              var selectedOptions = element.val();

              if(isArray(selectedOptions)) {
                $.each(selectedOptions, function(index, selected) {
                  elementValues.push(selected);
                });
              }
              else {
                elementValues.push(selectedOptions);
              }
            }
            else {
              elementValues.push(element.val());
            }
          }
        );
        
        return elementValues;
      }

      // Replace tokens of a given url.
      function webformPopupHelperReplaceUrlTokens(form, value) {
        value = decodeURIComponent(value);
        var url = value; 
        var tokenRegexp = new RegExp(/\[(submission:values:(.*?))\]/ig);
        var matches;
        while (matches = tokenRegexp.exec(value)) {
          var tokenString = matches[0];
          var fieldName = matches[2];
          var elementValues = webformPopupHelperGetElementValue(form, fieldName);
          elementValues = elementValues.join(' ');

          if (elementValues) {
            url = url.replace(tokenString, encodeURIComponent(elementValues));
          }
          else {
            url = url.replace(tokenString, '');
          }
        }
        
        return url;
      }

      // Test a webform field value against a conditional.
      function webformPopupHelperConditionalValueMatch(elementValue, conditionalValue) {
        var matches = conditionalValue.split(',');
        var matching = new Boolean(false);
        $.each(matches, function(i){
          if (elementValue == matches[i]) {
            matching = true;
          }
        });
        
        return matching;
      }

      // Process Popup urls.
      function webformPopupHelperProcessPopupUrls(formSubmitted, setting, emailExists) {
        // Check if defined urls match conditions.
        var urls = [];
        var useConditionalUrls = new Boolean(false);
        if (typeof setting.js_popup_settings.urls === 'object') {
          if (typeof setting.js_popup_settings.urls.conditionals !== 'undefined') {
            var urlConditionals = setting.js_popup_settings.urls.conditionals;
            if (urlConditionals.length > 0) {
              var firstConditional = new Boolean(false);
              $.each(urlConditionals, function(index, urlConditional) {
                var elementValues = webformPopupHelperGetElementValue(formSubmitted, urlConditional['component_name']);

                $.each(elementValues, function(index, elementValue) {
                  if (webformPopupHelperConditionalValueMatch(elementValue, urlConditional['values']) == true) {
                    if (firstConditional != false) {
                      urls.push(urlConditional['url']); 
                    }

                    useConditionalUrls = true;
                    firstConditional = true;
                  }
                });
              });
            }
          }
      
          // Use duplicates urls and default urls (if necessary).
          if (useConditionalUrls == false) {
            if (typeof setting.js_popup_settings.urls.duplicate !== 'undefined' && emailExists == true) {
              var urlDuplicates = setting.js_popup_settings.urls.duplicate;
       
              if (urlDuplicates.length > 0) {
                $.each(urlDuplicates, function(index, urlDuplicate) {
                  urls.push(urlDuplicate['url']); 
                });
              }
            }
            else if (typeof setting.js_popup_settings.urls.defaults !== 'undefined') {
              // Use default urls (if necessary).
              var urlDefaults = setting.js_popup_settings.urls.defaults;
        
              if (urlDefaults.length > 0) {
                $.each(urlDefaults, function(index, urlDefault) {
                  urls.push(urlDefault['url']); 
                });
              }
            }
          }
        }

        if (urls.length > 0) {
          $.each(urls, function(index, url) {
            // Replace tokens in url and open window popup.
            url = webformPopupHelperReplaceUrlTokens(formSubmitted, url);
            webformPopupHelperWindowOpen(url); 
          });
        }        
      }
  
      // Open window popup via Ajax to avoid browser's window blocking.
      function webformPopupHelperWindowOpen(url) {
        var ajaxUrl = Drupal.settings.basePath + 'webform-popup-helper/popup';
        var redirectWindow = window.open(url, '_blank');
        
        $.ajax({
          url: location.protocol + '//' + location.host + ajaxUrl,
          type: 'POST',
          dataType: 'json',
          success: function (data) {
            redirectWindow.location;
          }
        });  
      }    

      var forms = settings.webform_popup_helper;  
      $.each(forms, function(formId, form) {    
        // Get Form Id selector.
        var formIdSelector = '#' + formId.replace(/([_])+/g, '-');

        // Ensure that this behavior is only attached once to the window.
        $(context).find(formIdSelector).once('webform_popup_helper', function() {
          $(formIdSelector).on('submit', function (event) {
            // Validate the form, validate() is called automatically.
            var valid = $(formIdSelector).valid();

            // If validation fails, return.
            if (!valid) {
              return;
            } 			  
            
			var formSubmitted = $(this);
			var setting = forms[formId];
        
		    // Get form nid.
            var formNid = webformPopupHelperGetNodeId(formSubmitted);
            
			// Get email address.
			var emailAddress = webformPopupHelperGetElementValue(formSubmitted, 'email_address');
			      
            var ajaxUrl =  Drupal.settings.basePath + 'webform-popup-helper/check-email/' + formNid + '/' + emailAddress;

            $.ajax({
              url: location.protocol + '//' + location.host + ajaxUrl,
              dataType: 'jsonp',
              async: false,
              success: function(data) {
                webformPopupHelperProcessPopupUrls(formSubmitted, setting, data.email_exists);
              }
            });
          });
        });
      });
    }
  };
})(jQuery);