<?php

module_load_include('inc', 'fjm_clamor', 'displays/basic-display');

class ConcertCollectionDisplay extends BasicDisplay {
  public static function addTabsStatically($pid, $page = NULL) {
    $tabset = array();
    
    if (user_access('view fedora collection')) {
      module_load_include('inc', 'fedora_repository', 'ObjectHelper');
      module_load_include('inc', 'fedora_repository', 'CollectionClass');
      
      $collection_query = <<<END_QUERY
PREFIX fre: <info:fedora/fedora-system:def/relations-external#>
PREFIX fm: <info:fedora/fedora-system:def/model#>
PREFIX fv: <info:fedora/fedora-system:def/view#>
PREFIX dc: <http://purl.org/dc/elements/1.1/>
PREFIX atm-rel: <http://digital.march.es/atmusica#>
SELECT ?object ?title ?thumbnail
WHERE {
  ?object fre:isMemberOf <info:fedora/{$pid}> ;
         dc:title ?title ;
         fm:state fm:Active .
  OPTIONAL {
    ?thumbnail_obj atm-rel:isIconOf ?object ;
                   fm:state fm:Active ;
                   fv:disseminates ?thumbnail .
    ?thumbnail fv:disseminationType <info:fedora/*/TN> .
  }
}
END_QUERY;
      $results = ObjectHelper::performSparqlQuery($collection_query);
      
      dsm($collection_query, 'collection query');
      dsm($results, 'results');
      $tabset['view'] = array(
          '#type' => 'tabpage',
          '#title' => t('View'),
          '#tab_name' => 'view-content',
          'content' => CollectionClass::assembleCollectionView($results),
      );
    }
    
    //Allow editing of the Collection-level (really, there's just the title) Metadata
    if (user_access('edit fedora meta data')) {
      //$form = drupal_get_form('fedora_repository_edit_qdc_form', $pid, 'DC');
    
      $tabset['edit_metadata'] = array(
          '#type' => 'tabpage',
          '#title' => t('Edit Collection Metadata'),
          '#content' => $form,
          '#tab_name' => 'edit_metadata'
      );
    }
    
    //Allow addition of Concerts to the collection.
    if (user_access('ingest new fedora objects')) {
      //$form = drupal_get_form('fedora_repository_ingest_form', $pid);
    
      $tabset['edit_metadata'] = array(
          '#type' => 'tabpage',
          '#title' => t('Add New Concert'),
          '#content' => $form,
          '#tab_name' => 'edit_metadata'
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
