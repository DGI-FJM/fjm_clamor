<?php

abstract class BasicDisplay {
  protected $pid;
  protected $title;
  
  public function __construct($pid) {
    $this->pid = $pid;
  }
  
  public static function get_title($pid) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    $item = new Fedora_Item($pid);
    return $item->objectProfile->objLabel;
  }
  
  public static function addTabsStatically($pid, $page_number = NULL) {
    $tabset = array();
    
    $tabset['test'] = array(
      '#value' => '<span>This display still needs to be completed.</span>',
    );
    
    return $tabset;
  }
  
  public function addTabs($page = NULL) {
    return static::addTabsStatically($this->pid, $page);
  }
}