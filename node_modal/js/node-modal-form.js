
(function ($) {

  Drupal.behaviors.nodemodalFieldsetSummaries = {
    attach: function (context, settings) {
      // Provide the vertical tab summaries (node add/edit page).
      $('fieldset.node-modal-node-settings-form', context).drupalSetSummary(function (context) {
        var statusCheckbox = $('.form-item-node-modal-status input', context);
 
        if (statusCheckbox.is(':checked')) {
          return Drupal.t('Enabled');
        }

        return Drupal.t('Disabled');
      });

      // Provide the vertical tab summaries (content type edit page).
      $('fieldset#edit-node-modal', context).drupalSetSummary(function (context) {
        var vals = [];
        $('input[type=checkbox]', context).each(function () {
          if (this.checked && this.attributes['data-enabled-description']) {
            vals.push(this.attributes['data-enabled-description'].value);
          }
          else if (!this.checked && this.attributes['data-disabled-description']) {
            vals.push(this.attributes['data-disabled-description'].value);
          }
        });
        return vals.join(', ');
      });
    }
  };
})(jQuery);
