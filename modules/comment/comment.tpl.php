<?php

/**
 * @file
 * Default theme implementation for comments.
 *
 * Available variables:
 * - $author: Comment author. Can be link or plain text.
 * - $content: An array of comment items. Use render($content) to print them all, or
 *   print a subset such as render($content['field_example']). Use
 *   hide($content['field_example']) to temporarily suppress the printing of a
 *   given element.
 * - $created: Formatted date and time for when the comment was created.
 *   Preprocess functions can reformat it by calling format_date() with the
 *   desired parameters on the $comment->created variable.
 * - $changed: Formatted date and time for when the comment was last changed.
 *   Preprocess functions can reformat it by calling format_date() with the
 *   desired parameters on the $comment->changed variable.
 * - $new: New comment marker.
 * - $permalink: Comment permalink.
 * - $submitted: Submission information created from $author and $created during
 *   template_preprocess_comment().
 * - $picture: Authors picture.
 * - $signature: Authors signature.
 * - $status: Comment status. Possible values are:
 *   comment-unpublished, comment-published or comment-preview.
 * - $title: Linked title.
 * - $classes: String of classes that can be used to style contextually through
 *   CSS. It can be manipulated through the variable $classes_array from
 *   preprocess functions. The default values can be one or more of the following:
 *   - comment: The current template type, i.e., "theming hook".
 *   - comment-by-anonymous: Comment by an unregistered user.
 *   - comment-by-node-author: Comment by the author of the parent node.
 *   - comment-preview: When previewing a new or edited comment.
 *   The following applies only to viewers who are registered users:
 *   - comment-unpublished: An unpublished comment visible only to administrators.
 *   - comment-by-viewer: Comment by the user currently viewing the page.
 *   - comment-new: New comment since last the visit.
 * - $title_prefix (array): An array containing additional output populated by
 *   modules, intended to be displayed in front of the main title tag that
 *   appears in the template.
 * - $title_suffix (array): An array containing additional output populated by
 *   modules, intended to be displayed after the main title tag that appears in
 *   the template.
 *
 * These two variables are provided for context:
 * - $comment: Full comment object.
 * - $node: Node object the comments are attached to.
 *
 * Other variables:
 * - $classes_array: Array of html class attribute values. It is flattened
 *   into a string within the variable $classes.
 *
 * @see template_preprocess()
 * @see template_preprocess_comment()
 * @see template_process()
 * @see theme_comment()
 *
 * @ingroup themeable
 */
?>

<style>
.customized-content{
	margin-bottom: 10px;
	transition: 200ms;
	padding-right: 30px;
	box-shadow: 0px 15px 10px -15px rgba(0,0,0,0.4), -15px 0px 10px -15px rgba(0, 0, 0, 0.2);
}

.customized-content:hover{
	transition: 200ms;
	padding-left: 20px;
	box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.2), 0 6px 20px 0 rgba(0, 0, 0, 0.19);
}

.submitted{
	font-size: 12px;
        font-family: "Trebuchet MS", Helvetica, sans-serif;
	font-style: none;
	font-weight: 100;
}
</style>

<div class="<?php print $classes; ?> clearfix customized-content"<?php print $attributes; ?> style="padding-left:30px; padding-bottom: 10px;">
  <?php print $picture ?>

  <?php if ($new): ?>
    <span class="new"><?php print $new ?></span>
  <?php endif; ?>

  <?php print render($title_prefix); ?>
  <h3<?php print $title_attributes; ?> style="color:#151B54; font-weight: 100; font-size: 20px; font-family: \"Trebuchet MS\", Helvetica, sans-serif;"><?php print $title ?></h3>
  <?php print render($title_suffix); ?>

  <div class="submitted" style="font-family: \"Trebuchet MS\", Helvetica, sans-serif; font-size: 10px;">
    <?php print $permalink; ?>
    <?php print $submitted; ?>
  </div>

  <div class="content"<?php print $content_attributes; ?> style="border-left: 2px solid rgb(35, 82, 124); margin-left: -15px; padding-left: 13px; text-align: justify; margin-bottom: 20px; font-size: 18px; font-family: \"Trebuchet MS\", Helvetica, sans-serif;">
    <?php
      // We hide the comments and links now so that we can render them later.
      hide($content['links']);
      print render($content);
    ?>
    <?php if ($signature): ?>
    <div class="user-signature clearfix" style="font-family: \"Trebuchet MS\", Helvetica, sans-serif;">
      <?php print $signature ?>
    </div>
    <?php endif; ?>
  </div>
<font style="font-family: \"Trebuchet MS\", Helvetica, sans-serif; color: #151B54;">
  <?php print render($content['links']) ?>
</font>
</div>
