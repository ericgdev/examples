(function($) {
  Drupal.behaviors.node_modal = {
    attach: function (context, settings) {
      if (typeof settings.node_modal === 'undefined') {
        return;
      }    
    
      // Check if clientside_validation is enabled.
      if (typeof settings.node_modal.clientside_validation_enabled !== 'undefined' && settings.node_modal.clientside_validation_enabled == true) {
        // Get Form Id selector.
        var formId = settings.node_modal.webform_form_id;
        var formIdSelector = '.' + formId.replace(/([_])+/g, '-');      
  
        $(context).find(formIdSelector).once('node_modal', function() {    
          $(formIdSelector).on('submit', function (e) {
            // Validate the form, validate() is called automatically.
            var valid = $(formIdSelector).valid();

            // If validation fails, return.
            if (!valid) {
              return;
            }      

            // Set cookie to not open the modal after page reloading.
            var date = new Date();
            date.setTime(date.getTime() + (15 * 1000));           
            Cookies.set('modal', 'valid', { expires: date, path: "/" });
      
            // Close current modal.
            $('.modal').modal('hide');        
          });
        });
      }
      
      // Add Modal Popup.
      if (!Cookies.get('modal')) {
        var modal_id = $('#' + settings.node_modal.modal_id, context);
        var seconds_delay = settings.node_modal.seconds_delay * 1000;
        var backdrop_option = settings.node_modal.closing_disabled == false ? true : 'static';

        setTimeout(function(){
          modal_id.modal({
            backdrop: backdrop_option
          });
        }, seconds_delay);
      }
    }
  };
})(jQuery);