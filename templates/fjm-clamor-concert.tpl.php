<?php 
drupal_set_title(t('Concert: @title', array('@title' => $concert['title'])));
?>
<div class="islandora_fjm_concert">
  <!-- <h2><?php echo $concert['title']; ?></h2>  -->
  <div class="atm_concert_top">
    <div class="leftcolumn">
      <div class="block">
      <?php echo theme('fjm_clamor_imagegallery', $pid) ?>
      </div>
    </div>
    <div class="rightcolumn">
      <div class="block">
        <div class="islandora_fjm_description">
          <h3 class="atm_concert_cycle"><?php echo $concert['cycle'] ?></h3>
          <p class="atm_concert_date"><?php echo format_date($concert['date']->format('U'), 'custom', 'd/m/Y') ?></p>
          <p class="atm_concert_description"><?php echo $concert['description'] ?></p>
        </div>
      </div>
   </div><!--atm_con_top_right -->
   <div class="clearfix"></div>
  </div>
  <div class="atm_concert_mid">
    <div class="leftcolumn">
      <div class="block">
      <h3><?php 
         if ($concert['program']['pid']) { 
           echo l(t('Program PDF Available'), 'fedora/repository/' . $concert['program']['pid'], array('attributes' => array('class' => 'pdf')));
         }
         else {
           echo t('N/A');
         }
       ?></h3>
      </div>
    </div>
    <div id="flowplayer" class="rightcolumn">
      <div class="block">
      <?php 
      //TODO:  Need to (better) determine whether or not to show the player...  Or just always show it?
      if (sizeof($concert['performance_rows']) + sizeof($concert['lecture_rows']) > 0)
      {
          echo theme('fjm_clamor_flowplayer');
      }
      ?>
      </div>
    </div>
  </div> <!-- atm_con_mid -->
  <div class="atm_concert_bottom">
    <div class="leftcolumn">
      <div class="block">
        <h3><?php echo t('Voice archive'); ?></h3>
        <?php 
          if (sizeof($concert['lecture_rows']) > 0) {
            echo theme('table', $concert['headers']['lecture'], $concert['lecture_rows'], array('class' => 'atm_concert_lecture_table'));
          }
          else {
            echo theme('table', array(), array(array(t('No items'))), array('class' => 'atm_concert_lecture_table'));
          }
        ?>
      </div>
    </div>
    <div class="rightcolumn">
      <div class="block">
        <h3><?php echo t('Works'); ?></h3>
        <?php
          if (sizeof($concert['performance_rows']) > 0) {
            echo theme('table', $concert['headers']['performance'], $concert['performance_rows'], array('class' => 'atm_concert_performance_table'));
          }
          else {
            echo theme('table', array(), array(array(t('No items'))), array('class' => 'atm_concert_performance_table'));
          }
        ?>
      </div>
    </div><!--bottom right-->
    <div class="clearfix"></div>
  </div>
  <?php
if ($pagenumber != NULL && $pagenumber > 0) :
  drupal_add_js(<<<ENDJS
\$(function() {
  \$f().onLoad(function() {
    \$("div.atm_track.concertOrder_$pagenumber:first > a").click();
  });
});
ENDJS
, 'inline', 'footer');
endif;?>
</div>
