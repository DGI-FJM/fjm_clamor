<?php

abstract class BasicDisplay {
  protected $pid;
  
  public function __construct($pid) {
    $this->pid = $pid;
  }
}