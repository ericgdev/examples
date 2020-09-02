(function($) {
  Drupal.behaviors.webform_input_effect = {
    attach: function (context, settings) {
      function checkForInput(element) {
        if ($(element).val().length == 0) {
          $(element).addClass('input-empty');
        } 
        else {
          $(element).removeClass('input-empty');
        }
      }

      $('.webform-client-form .form-item input').each(function() {
        checkForInput(this);
      }); 

      $('.webform-client-form .form-item input').on('change keyup', function() {
        checkForInput(this);  
      });  
    }
  };
})(jQuery);