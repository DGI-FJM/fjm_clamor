<?php

function fjm_clamor_composer_autocomplete($string = '') {
  module_load_include('inc', 'islandora_solr_search', 'IslandoraSolrQueryProcessor');
  //Perform Solr search on both score titles and composer names (make edge'd fields!)
  $qp = new IslandoraSolrQueryProcessor();
  $qp->solrParams = array(
    'defType' => 'dismax',
    'qf' => 'PID eaccpf_complete_et',
    'fq' => array(
      'rels_hasModel_ms:"atm:personCModel"',
    ),
    'rows' => 10,
    'fl' => 'PID,eaccpf_complete_es',
  );
  $qp->buildAndExecuteQuery($string);

  $results = array();
  foreach ((array)$qp->solrResult->response->docs as $doc) {
    $results[$doc->PID] = t('@pid: %composer', array(
      '@pid' => $doc->PID,
      '%composer' => $doc->eaccpf_complete_es,
    ));
  }
  drupal_json($results);
  exit();
}

module_load_include('inc', 'fjm_clamor', 'displays/basic-display');

class ScoreCollectionDisplay extends BasicDisplay {
  public static function addTabsStatically($pid, $page_number = NULL)
  {
    $tabset = array();
    
    if (user_access('view fedora collection')) {
      $collection_query = <<<END_QUERY
PREFIX fm: <info:fedora/fedora-system:def/model#>
PREFIX atm-rel: <http://digital.march.es/atmusica#>
SELECT ?object ?title ?thumbnail
FROM <#ri>
WHERE {
  ?object fm:hasModel <info:fedora/atm:scoreCModel> ;
          fm:state fm:Active ;
          fm:label ?title .
  OPTIONAL {
    ?thumbnail atm-rel:isIconOf ?object ;
               fm:state fm:Active .
  }
}
ORDER BY ASC(?title)
END_QUERY;
      module_load_include('inc', 'fedora_repository', 'CollectionClass');
      module_load_include('inc', 'fedora_repository', 'ObjectHelper');
      $results = ObjectHelper::performSparqlQuery($collection_query);
      $tabset['view'] = array(
        '#type' => 'tabpage',
        '#title' => t('View'),
        '#tab_name' => 'view-collection',
        'content' => CollectionClass::assembleCollectionView($results),
      );
      
    }
    
    if (user_access('ingest new fedora objects')) {
      $tabset['add-score'] = array(
        '#type' => 'tabpage',
        '#title' => t('Add'),
        '#tab_name' => 'add-score',
        '#content' => drupal_get_form('fedora_repository_ingest_form', $pid, self::get_title($pid), 'atm:scoreCModel'),
      );
    }
    return $tabset;
  }
}