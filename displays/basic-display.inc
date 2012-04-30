<?php

abstract class BasicDisplay {
  protected $pid;
  
  public function __construct($pid) {
    $this->pid = $pid;
  }
  
  public static function addTabsStatically($pid, $page_number = NULL) {
    $tabset = array();
    
    $tabset['test'] = array(
      '#value' => '<span>blargh</span>',
    );
    
    return $tabset;
  }
  
  public function addTabs($page = NULL) {
    return self::addTabsStatically($this->pid, $page);
  }
}