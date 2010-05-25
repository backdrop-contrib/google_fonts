<?php
// $Id$

/**
 * @file
 * Administrative page callbacks for the Google Fonts module.
 */

/**
 * Implementation of hook_admin_settings() for configuring the module
 */
function google_fonts_admin_settings_form(&$form_state) {

  $form = array();
  $form['introduction'] = array(
    '#value' => t('Select the fonts that you want to be available on your website. Keep in mind that each font takes some time to download. For the best performance, only request the fonts that you are using on your website.'),
  );
  
  $available_fonts = array();
  foreach (_google_fonts_available_fonts() as $font) {
    $available_fonts[_google_fonts_system_fontname($font)] = '<span class="font_example ' . _google_fonts_system_fontname($font) . '">' . $font . '</span>';
  }
  
  $style_to_add = '<style type="text/css"> #google-fonts-admin-settings-form .form-item{ margin-bottom: 20px; } span.font_example { font-size: 30px; vertical-align: middle; }';
  foreach (_google_fonts_available_fonts() as $font) {
    $style_to_add .= '.' . _google_fonts_system_fontname($font) . '{ font-family: "' . $font . '"; } ';
  }
  $style_to_add .= '</style>';
  drupal_set_html_head($style_to_add);
  
  /* Load all fonts in CSS to display their example */
  foreach (_google_fonts_available_fonts() as $font) {
      $fonts_to_load[] = _google_fonts_google_fontname($font);
  }
  _google_fonts_load_css($fonts_to_load);
  
  $form['google_fonts_enabled_fonts'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Enabled Google Fonts'),
    '#default_value' => variable_get('google_fonts_enabled_fonts', array()),
    '#options' => $available_fonts,
    '#description' => t('After saving you can use the enabled fonts in your stylesheet.<br />Example: <code>.node h2{ font-family: "Josefin Sans Std Light"; }</code>'),
  );
  
  return system_settings_form($form);
}