<?php

/**
 * @file
 * Default theme implementation to display the Modap Popup configured for the node.
 *
 * Available variables:
 * - $modal_id: Modal Id.
 * - $modal_class: Modal Class.
 * - $header_text: Header text.
 * - $content: Display Content selected.
 * - $closing_disabled: Determine if closing option is available or not.
 *
 * @see template_preprocess_node_modal()
 *
 * @ingroup themeable
 */
?>
<?php if ($content || $header_text): ?>
  <div id="<?php print $modal_id; ?>" class="modal fade <?php print $modal_class; ?>" tabindex="-1" role="dialog" aria-labelledby="myModalLabel" aria-hidden="true">
    <div class="modal-dialog modal-lg" role="document">
      <div class="modal-content">
        <?php if (!$closing_disabled): ?>
          <div class="modal-header">
            <button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
          </div>
        <?php endif; ?>
        <div class="modal-body">
          <?php if ($header_text): ?>
            <div class="header-text"><?php print $header_text; ?></div>
          <?php endif; ?>

          <?php if ($content): ?>
            <div class="content-wrapper"><?php print $content; ?></div>
          <?php endif; ?>
        </div>
        <?php if (!$closing_disabled): ?>
          <div class="modal-footer">
            <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
          </div>
        <?php endif; ?>
      </div>
    </div>
  </div>
<?php endif; ?>