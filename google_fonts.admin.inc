<?php

/**
 * @file
 * Administrative page callbacks for the Google Fonts module.
 */

/**
 * Implements hook_admin_settings() for configuring the module
 */
function google_fonts_admin_settings_form($form, &$form_state) {

  $form = array();
  $form['introduction'] = array(
    '#markup' => t('Select the fonts that you want to be available on your website. Keep in mind that each font takes some time to download. For the best performance, only enable fonts that you are actually using on your website. More information about these fonts is available on the <a href="!link">Google font directory</a>.', array('!link' => 'http://code.google.com/webfonts')),
  );
  
  $available_fonts = array();
  foreach (_google_fonts_available_fonts() as $font_family => $fonts) {
    foreach ($fonts['fonts'] as $font_name => $font_info) {
      $available_fonts[$font_name] = '<span class="font_example ' . _google_fonts_path_to_classname($font_info['path']) . '">' . $font_name . '</span>';
    }
  }
  
  /* Load all fonts in CSS to display their example */
  $style_to_add = '#google-fonts-admin-settings-form .form-item{ margin-bottom: 20px; } span.font_example { font-size: 30px; vertical-align: middle; cursor: pointer; padding-left:10px; }' . PHP_EOL;
  foreach (_google_fonts_available_fonts() as $font_family => $fonts) {
    foreach ($fonts['fonts'] as $font_name => $font_info) {
      $fonts_to_load[] = $font_info['path'];
      $style_to_add .= '.' . _google_fonts_path_to_classname($font_info['path']) . '{ font-family: "' . $font_info['family'] . '"; font-style: ' . $font_info['style'] . '; font-weight: ' . $font_info['weight'] . '; } ' . PHP_EOL;
    }
  }
  _google_fonts_load_css($fonts_to_load);
  drupal_add_css($style_to_add, 'inline');

  /* Create the enabled_fonts array to be used by the fieldset that displays the enabled fonts and remove the enabled fonts from the available_fonts array */
  $enabled_fonts = array();
  $enabled_fonts_variable = array_filter(variable_get('google_fonts_enabled_fonts', array()), "_google_fonts_filter_enabled_fonts");
  $available_fonts_keys = array_keys($available_fonts);
  foreach ($available_fonts_keys as $key) {
    if (in_array($key, $enabled_fonts_variable)) {
      $enabled_fonts[$key] = $available_fonts[$key];
      unset($available_fonts[$key]);
    }
  }
  
  $available_fonts_collapsed = TRUE;
  /* Check if there are any enabled fonts, otherwise don't create the fieldset and display a message*/
  if (!empty($enabled_fonts_variable)) {
    $form['google_fonts_fieldset_enabled_fonts'] = array(
      '#type' => 'fieldset',
      '#title' => t('Enabled Google Fonts'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    );
  
    $form['google_fonts_fieldset_enabled_fonts']['google_fonts_enabled_fonts'] = array(
      '#type' => 'checkboxes',
      '#default_value' => $enabled_fonts_variable,
      '#options' => $enabled_fonts,
      '#description' => t('You can use the enabled fonts in your stylesheet.<br />Example: <code>.node h2{ font-family: "Josefin Sans Std Light"; }</code>'),
    );
  }
  else {
    $available_fonts_collapsed = FALSE;
    drupal_set_message(t('No Google Fonts have been enabled yet. Please enable one or more Google Fonts below.'), 'status');
  }
  
  /* Check if there are any available fonts that have nog been enabled yet, otherwise don't create the fieldset and display a message */
  if (!empty($available_fonts)) {
    $form['google_fonts_fieldset_available_fonts'] = array(
      '#type' => 'fieldset',
      '#title' => t('Available Google Fonts'),
      '#collapsible' => TRUE,
      '#collapsed' => $available_fonts_collapsed,
    );  
    
    $form['google_fonts_fieldset_available_fonts']['google_fonts_available_fonts'] = array(
      '#type' => 'checkboxes',
      '#default_value' => array(),
      '#options' => $available_fonts,
    );
  }
  else {
    drupal_set_message(t('All available Google Fonts have been enabled.'), 'status');
  }
  
  /* Only provide a submit button if there are any fields to submit (highly theoretical situation for this module, because this will only be the case if there are no Google Fonts to select). ;) */
  if (!empty($enabled_fonts_variable) || !empty($available_fonts)) {
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save configuration'),
      '#weight' => 5,
    );
  }
  
  return $form;
}

/**
 * Google fonts admin settings form submission function.
 */
function google_fonts_admin_settings_form_submit($form, &$form_state) {
  /* Create the settings to save from the two fields that could be submitted by merging them or using either one of them if they are set. */
  if(isset($form_state['values']['google_fonts_enabled_fonts']) && isset($form_state['values']['google_fonts_available_fonts'])) {
    $enabled_fonts = array_merge($form_state['values']['google_fonts_enabled_fonts'], $form_state['values']['google_fonts_available_fonts']);
  }
  else {
    if(isset($form_state['values']['google_fonts_enabled_fonts'])) {
      $enabled_fonts = $form_state['values']['google_fonts_enabled_fonts'];
    }
    else {
      $enabled_fonts = $form_state['values']['google_fonts_available_fonts'];
    }
  }
  variable_set('google_fonts_enabled_fonts', $enabled_fonts);
  
  /* Somehow earlier displayed messages are displayed again. To prevent this get all the status messages to clear them. */
  drupal_get_messages('status');
  drupal_set_message(t('Configuration saved'), 'status');
}

/**
 * Filter function to filter out the disabled Google Fonts.
 */
function _google_fonts_filter_enabled_fonts($value) {
  if ($value == '0') {
    return FALSE;
  }
  return TRUE;
}