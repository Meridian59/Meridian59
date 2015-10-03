jQuery( document ).ready(function($) {
    /**
     *  Search guild Table and Build Results List
     */
    $guildResults = $( "#guild-results" );
    var badge = '';
    $("#guild-search-bar").on('keyup', '#guild-search', function(e){
        if($(this).val().length > 2){
            $.post( "ajax/get-guilds.php", { name: $(this).val() }, function( response ) {
                $guildResults.html('');
                if(response.status == "success"){

                    for(var p = 0; p < response.data.length; p++){
                        badge = '';
                        if(response.data[p].suicide == 1){
                            badge = '<span class="badge danger">S</span>';
                        }

                        $guildResults.append(  '<a href="#" data-id="'+response.data[p].id+'" class="list-group-item guild-result">'+
                                                    '<h4 class="list-group-item-heading">'+response.data[p].name+' '+badge+'</h4>'+
                                                '</a>');
                    }
                }
            });
        }
    });

    /**
     *  Build guild Information
     */

    $("#guild-results").on('click', 'a.guild-result', function(e){
        e.preventDefault();
        if($(this).attr('data-id') != ""){
            $.post( "ajax/get-guild.php", { idguild: $(this).attr('data-id') }, function( response ) {
                if(response.status == "success"){
                    var guild = response.data.guild;

                    /* REVEAL INFORMATION */
                    $('#guild-information .panel').show();
                    $('.guild_name').html(guild.guild_name);

                    /* DISBAND CHECK */
                    if(guild.guild_disbanded == 1){
                        $('#suicide-alert').removeClass('hidden');
                        $('#suicide-alert .guild_disbanded_time').html(guild.guild_disbanded_time);
                    }else{
                        $('#suicide-alert').addClass('hidden');
                    }

                    /* IDENTIFICATION */
                    $('#guild_name').html(guild.guild_name);
                    $('#guild_leader').html(guild.guild_leader);
                    $('#guild_hall').html(guild.guild_hall);

                }
            });
        }
    });

    /**
     *  Open Stat Windows
     */
    $('#guild-information').on('click', '.panel-heading', function(e){
        $(this).next().toggle();
    });

    /**
     *  Toggle Death Information
     */
    $('#guild-deaths').on('click', 'h4', function(e){
        $(this).next().toggle();
    });
});
