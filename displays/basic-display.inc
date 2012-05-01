<?php

abstract class BasicDisplay {
  protected $pid;
  
  public function __construct($pid) {
    $this->pid = $pid;
  }
  
  public static function addTabsStatically($pid, $page_number = NULL) {
    $tabset = array();
    
    $tabset['test'] = array(
      '#value' => '<span>This display still needs to be completed.</span>',
    );
    
    return $tabset;
  }
  
  public function addTabs($page = NULL) {
    return self::addTabsStatically($this->pid, $page);
  }
}