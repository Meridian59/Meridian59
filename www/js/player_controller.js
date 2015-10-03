jQuery( document ).ready(function($) {
    /**
     *  Search Player Table and Build Results List
     */
    $playerResults = $( "#player-results" );
    var badge = '';
    $("#player-search-bar").on('keyup', '#player-search', function(e){
        if($(this).val().length > 2){
            $.post( "ajax/get-players.php", { name: $(this).val() }, function( response ) {
                $playerResults.html('');
                if(response.status == "success"){

                    for(var p = 0; p < response.data.length; p++){
                        badge = '';
                        if(response.data[p].suicide == 1){
                            badge = '<span class="badge danger">S</span>';
                        }

                        $playerResults.append(  '<a href="#" data-id="'+response.data[p].id+'" class="list-group-item player-result">'+
                                                    '<h4 class="list-group-item-heading">'+response.data[p].name+' '+badge+'</h4>'+
                                                '</a>');
                    }
                }
            });
        }
    });

    /**
     *  Build Player Information
     */

    $("#player-results").on('click', 'a.player-result', function(e){
        e.preventDefault();
        if($(this).attr('data-id') != ""){
            $.post( "ajax/get-player.php", { idplayer: $(this).attr('data-id') }, function( response ) {
                if(response.status == "success"){
                    var player = response.data.player;
                    var logins = response.data.logins;
                    var deaths = response.data.deaths;

                    /* REVEAL INFORMATION */
                    $('#player-information .panel').show();
                    $('.player_name').html(player.player_name);

                    /* SUICIDE CHECK */
                    if(player.player_suicide == 1){
                        $('#suicide-alert').removeClass('hidden');
                        $('#suicide-alert .player_suicide_time').html(player.player_suicide_time);
                    }else{
                        $('#suicide-alert').addClass('hidden');
                    }

                    /* IDENTIFICATION */
                    $('#player_account_id').html(player.player_account_id);
                    $('#player_name').html(player.player_name);
                    $('#player_home').html(player.player_home);
                    $('#player_guild').html(player.player_guild);
                    $('#player_last_login').html(player.player_last_login);

                    /* STATS */
                    $('#player_max_health h5 span').html(player.player_max_health);
                    $('#player_max_health .progress-bar ').css('width',(parseInt(player.player_max_health) / 150)*100+'%');
                    $('#player_max_mana h5 span').html(player.player_max_mana);
                    $('#player_max_mana .progress-bar ').css('width',(parseInt(player.player_max_mana) / 150)*100+'%');
                    $('#player_might h5 span').html(player.player_might);
                    $('#player_might .progress-bar ').css('width',(parseInt(player.player_might) / 50)*100+'%');
                    $('#player_int h5 span').html(player.player_int);
                    $('#player_int .progress-bar ').css('width',(parseInt(player.player_int) / 50)*100+'%');
                    $('#player_stam h5 span').html(player.player_stam);
                    $('#player_stam .progress-bar ').css('width',(parseInt(player.player_stam) / 50)*100+'%');
                    $('#player_agil h5 span').html(player.player_agil);
                    $('#player_agil .progress-bar ').css('width',(parseInt(player.player_agil) / 50)*100+'%');
                    $('#player_myst h5 span').html(player.player_myst);
                    $('#player_myst .progress-bar ').css('width',(parseInt(player.player_myst) / 50*100)+'%');
                    $('#player_aim h5 span').html(player.player_aim);
                    $('#player_aim .progress-bar ').css('width',(parseInt(player.player_aim) / 50)*100+'%');

                    /* LOGINS  */
                    $playerLoginsTable = $('#player-logins tbody');
                    $playerLoginsTable.html('');

                    for(var i = 0; i < logins.length; i++){
                        $playerLoginsTable.append('<tr><td>'+logins[i].player_logins_time+'</td><td>'+logins[i].player_logins_IP+'</td></tr>')
                    }

                    /* DEATHS  */
                    $playerDeathsTable = $('#player-deaths .panel-body');
                    $playerDeathsTable.html('');

                    for(var i = 0; i < deaths.length; i++){
                        $playerDeathsTable.append(  '<div class="death-wrapper"><h4>'+deaths[i].player_death_time+'</h4>'+
                                                    '<table class="table"><thead><tr>'+
                                                            '<th>Killer</th><th>Location</th><th>Killing Blow</th>'+
                                                    '</tr></thead><tbody>'+
                                                    '<tr class="">'+
                                                        '<td>'+deaths[i].player_death_killer+'</td><td>'+deaths[i].player_death_room+'</td><td>'+deaths[i].player_death_attack+'</td>'+
                                                    '</tr></tbody></table></div>')
                    }
                }
            });
        }
    });

    /**
     *  Open Stat Windows
     */
    $('#player-information').on('click', '.panel-heading', function(e){
        $(this).next().toggle();
    });

    /**
     *  Toggle Death Information
     */
    $('#player-deaths').on('click', 'h4', function(e){
        $(this).next().toggle();
    });
});
