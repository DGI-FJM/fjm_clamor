<?php

module_load_include('inc', 'fjm_clamor', 'displays/basic-display');

class PersonDisplay extends BasicDisplay {
  public static function addTabsStatically($pid, $page = NULL) {
    $tabset = array(
      '#type' => 'tabset',
    );
  
    if (user_access('view fedora collection')) {
      $tabset['view-content'] = array(
        '#type' => 'tabpage',
        '#title' => t('View'),
        '#tab_name' => 'view-content',
        '#content' => theme('fjm_clamor_person', $pid, $page),
      );
    }
    
    if (user_access('ingest new fedora objects')) {
      $tabset += self::getImageTab($pid);
    }
  
    return $tabset;
  }
}
