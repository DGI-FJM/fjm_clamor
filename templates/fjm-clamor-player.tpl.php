<?php  
$trackingCode = variable_get('googleanalytics_account', NULL); ?>

<div class="player" id="atm_player"></div>
<ol class="atm_clips" style="display: none;"></ol>
<script type="text/javascript">
  function update_ga(event, url, time) {
    <?php
//FIXME:  Is this custom stuff necessary?  There now seems to be an analytics 
//  plugin provided by the folks at Flowplayer
    if(!empty($trackingCode)): ?>
    if (typeof _gaq != "undefined") {
      if (time != null) {
        _gaq.push(['_trackEvent', 'Audio', event, url, time])
      }
      else {
        _gaq.push(['_trackEvent', 'Audio', event, url]);
      }
    }
    <?php endif; ?>
  }

  $(function() {
    $f("atm_player", "<?php echo $base; ?>/flowplayer-3.2.7.swf", {
      plugins: {
        controls: {
          all: false,
          play: true,
          playlist: true,
          volume: true,
          autoHide: false,
          scrubber: true,
          time: true,
          height: 30
        },
        audio: {
          url: "<?php echo $base; ?>/flowplayer.audio-3.2.2.swf"
        },
        content: {
          url: "<?php echo $base; ?>/flowplayer.content-3.2.0.swf",
          stylesheet: "<?php echo drupal_get_path('module', 'fjm_clamor'); ?>/css/fjm-clamor-player.css",
          width: '95%',
          height: '60%'
        }
      },
      clip: {
        baseUrl: "<?php echo $base_url; ?>",
        autoPlay: false,
        autoBuffering: true,
        onStop: function(clip) {
          update_ga('Stop', clip.url, parseInt(this.getTime()));
        },
        onPause: function(clip) {
          update_ga('Pause', clip.url, parseInt(this.getTime()));
        },
        onFinish: function(clip) {
          update_ga('Finish', clip.url, null);
          if (clip.index + 1 == this.getPlaylist().length) {
            //Make it start the next playlist on the last piece...
            Drupal.settings.islandora_fjm.current_index += 1;
            if (Drupal.settings.islandora_fjm[Drupal.settings.islandora_fjm.current_type].length > Drupal.settings.islandora_fjm.current_index) {
              this.setPlaylist(Drupal.settings.islandora_fjm[Drupal.settings.islandora_fjm.current_type][Drupal.settings.islandora_fjm.current_index]).play(0);
            }
            else {
              this.stop();
            }
          }
          else {
            this.play(clip.index + 1);
          }
        },
        onStart: function(clip) {
          update_ga('Play', clip.url, null);
        },
        subTitle: ''
      },
      onPlaylistReplace: function(clips){ 
        $("<?php echo $selector; ?>").show();
        if (clips.length > 0 && typeof clips[0].now_playing != 'undefined') {
          this.getPlugin('content').setHtml(clips[0].now_playing);
        }
      },
      onLoad: function() {
        var types = ['lecture', 'audio', 'piece'];
        $.each(types, function(index, value) {
          if (typeof Drupal.settings.islandora_fjm[value] != 'undefined') {   
            var playlist = Drupal.settings.islandora_fjm[value];
            if (playlist.length > 0) {
              Drupal.settings.islandora_fjm.current_type = value;
              Drupal.settings.islandora_fjm.current_index = 0;
              $f().setPlaylist(playlist[0]);
            }
          }
        });
      }
    });

    $f("atm_player").playlist("<?php echo $selector; ?>", {
      loop: true,
      template: "<li><a href=\"${url}\">${title}</a></li>",
      manual: false
    });
   });
  
  Drupal.settings.islandora_fjm.play = function (type, piece_index, movement_index) {
    Drupal.settings.islandora_fjm.current_type = type; 
    Drupal.settings.islandora_fjm.current_index = piece_index;
    var piece = Drupal.settings.islandora_fjm[type][piece_index];
    $f().setPlaylist(piece).play(movement_index);
  };
</script>
