<?php

/**
 * @file
 * Class containing ISLANDORACM callbacks used in Clamor.
 */
class fjm_hooks {
  public $pid;
  
  function __construct($pid) {
      $this->pid = $pid;
  }
  
  /**
   * Pagenumber isn't really used...
   * Should I actually check the content model?
   * 
   * TODO:  Might want to make it verify the content model.
   * @deprecated
   */
  function showPerson($pagenumber = NULL) {
    return theme('fjm_clamor_person', $this->pid);
  }
  
  /**
   *
   * @param $pagenumber string
   *   Used as a bit of a hack to determine which song to start playing, if set.
   * @return String 
   */
  function showConcert($pagenumber = NULL) {
    //return "<span>lol</span>";
    //return theme('fjm_clamor_concert', $this->pid, $pagenumber);
  }
  
  function showScore($pagenumber = NULL) {
    //return theme('fjm_clamor_score', $this->pid);
  }
  
  function showProgram($pagenumber = NULL) {
    //return theme('fjm_clamor_program', $this->pid);
  }
}
