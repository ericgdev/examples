(function($) {
  Drupal.behaviors.multilink = {
    attach: function (context, settings) {
      // Open window popup via Ajax to avoid browser's window blocking.
      function multilink_window_open(url) {
        var ajax_url = Drupal.settings.basePath + 'multilink/check';
        var redirectWindow = window.open(url, '_blank');
        $.ajax({
          url: location.protocol + '//' + location.host + ajax_url,
          type: 'POST',
          dataType: 'json',
          success: function (data) {
            redirectWindow.location;
          }
        });
      }

      $('.field-multilink').click(function(e) {
        var multilink_classes = $(this).attr('class').split(' ');
        var field_id_class = multilink_classes[1];

        // Get passed parameters.
        var setting = settings.multilink[field_id_class];
        var seconds_delay = setting.seconds_delay * 1000;
        var window_urls = setting.window_urls;

        $.each(window_urls, function(index, url){
          multilink_window_open(url); 
        });

        var rollunder_url = setting.rollunder_url;

        if (seconds_delay > 0 && rollunder_url) {
          setTimeout(function() { 
            window.location.href = rollunder_url;
          }, seconds_delay);
        }
      });
    }
  };

})(jQuery);