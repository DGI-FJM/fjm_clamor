<?php

module_load_include('inc', 'fjm_clamor', 'displays/basic-display');

class ConcertCollectionDisplay extends BasicDisplay {
  /**
   * Get the query to return all the objects in this collection.
   * 
   * @param $pid string
   *   A string containing the PID of the collection object.
   * @return string
   *   A string containing a Sparql query to obtain a set of results, each
   *   comprising a child "object", it's "title" and optionally a PID of a
   *   "thumbnail" datastream (that is, something like <info:fedora/PID/DSID>;
   *   the "info:fedora/" chunk should be stripped, and then the PID/DSID chunk
   *   can be appended to create a path like "fedora/repository/PID/DSID", to
   *   actually use the thumbnail. 
   */
  protected static function getCollectionQuery($pid) {
    //Get all concerts which are a member of this cycle/series, ordered by title.
    return <<<END_QUERY
PREFIX fre: <info:fedora/fedora-system:def/relations-external#>
PREFIX fm: <info:fedora/fedora-system:def/model#>
PREFIX fv: <info:fedora/fedora-system:def/view#>
PREFIX dc: <http://purl.org/dc/elements/1.1/>
PREFIX atm-rel: <http://digital.march.es/atmusica#>
SELECT ?object ?title ?thumbnail
WHERE {
  ?object fre:isMemberOf <info:fedora/{$pid}> ;
          dc:title ?title ;
          fm:state fm:Active ;
          fm:hasModel <info:fedora/atm:concertCModel> .
  OPTIONAL {
    ?thumbnail_obj atm-rel:isIconOf ?object ;
                   fm:state fm:Active ;
                   fv:disseminates ?thumbnail .
    ?thumbnail fv:disseminationType <info:fedora/*/TN> .
  }
}
ORDER BY ASC(?title)
END_QUERY;
  }
  
  /**
   * Get an array to create a tabpage containing the collection.
   * 
   * @param $pid string
   *   A string containing the PID of this item.
   * @return array
   *   An array representing a tabpage.  
   */
  protected static function getCollectionTab($pid) {
    module_load_include('inc', 'fedora_repository', 'ObjectHelper');
    module_load_include('inc', 'fedora_repository', 'CollectionClass');
    
    $results = ObjectHelper::performSparqlQuery(self::getCollectionQuery($pid));
    
    return array(
        '#type' => 'tabpage',
        '#title' => t('View'),
        '#tab_name' => 'view-content',
        'content' => CollectionClass::assembleCollectionView($results),
    );
  }
  
  /**
   * Get the tabs for this content model display.
   * 
   * @param $pid string
   *   A string containing the pid of this object.
   * @param $page integer
   *   An integer representing which page should be shown (not really used, in
   *   favor of Drupal-native paging.
   * @return array
   *   An array of arrays representing a set of tabpages.
   */
  public static function addTabsStatically($pid, $page = NULL) {
    $tabset = array();
    
    if (user_access('view fedora collection')) {
      $tabset['view'] = self::getCollectionTab($pid);
    }
    
    //Allow addition of Concerts to the collection.
    if (user_access('ingest new fedora objects')) {
      $form = drupal_get_form('fedora_repository_ingest_form', $pid, self::get_title($pid), 'atm:concertCModel');
    
      $tabset['ingest-concert'] = array(
          '#type' => 'tabpage',
          '#title' => t('Add New Concert'),
          '#content' => $form,
          '#tab_name' => 'ingest-concert',
      );
    }
    
    return $tabset;
  }
  
  
  
  /* TODO:  Finish... Then again, is this even a good idea (instead of the odd theme implementation)?...  Iunno.
  private function getConcertMarkupArray($piece_index = NULL) {
    $title = 'get the title';
    return array(
      '#type' => 'markup',
      '#prefix' => '<div class="islandora_fjm_concert">',
      '#suffix' => '</div>',
      'title' => array(
        '#prefix' => '<h3>',
        '#suffix' => '</h3>',
        '#value' => $title,
      ),
      'top' => array(
      
      ),
      'bottom' => array(
      
      ),
    );
  }*/
}
