//TODO: build a return data function...?
//TODO: build a display chart function...?

jQuery( document ).ready(function($) {

    /**
     * Charts and Canvas Functions
     */

    function m59DrawChart(chartType, chartData){
        // Reset the chart space. This was the only sure way I found to do it
        $('#canvas-container').html('<canvas id="canvas" height="500" width="900"></canvas>');
        var canvasCxt = document.getElementById("canvas").getContext("2d");
        var serverChart = new Chart(canvasCxt);

        //clean up any current charts
        switch(chartType){
            case 'bar':
                serverChart.Bar(chartData, {responsive : true});
                break;
            case 'line':
                serverChart.Line(chartData, {responsive : true});
                break;
        }
    }

    /**
     *  Display Avg PvP Damage
     */

    $("#server-menu").on('click', 'a#avgpvpdamage', function(e){
        e.preventDefault();
        if($(this).attr('data-stat') != ""){
            $.post( "ajax/get-"+$(this).attr('data-stat')+".php", function( response ) {
                if(response.status == "success"){

                    var chartData = response.data,
                        labelsArray = [],
                        datasetData = [];

                    for(var i = 0; i < chartData.length; i++){
                        labelsArray.push(chartData[i].weapon);
                        datasetData.push(parseInt(chartData[i].pvpdamagetaken));
                    }

                    var barChartData = {
                                        labels : labelsArray,
                                        datasets : [{
                                        fillColor : "rgba(100,100,220,0.5)",
                                        strokeColor : "rgba(220,220,220,0.8)",
                                        highlightFill: "rgba(220,220,220,0.75)",
                                        highlightStroke: "rgba(220,220,220,1)",
                                        data : datasetData
                                        }]
                    }

                    m59DrawChart('bar', barChartData);

                }
            });
        }
    });

    /**
     *  Display Money Time Daily
     */

    $("#server-menu").on('click', 'a#moneytimedaily', function(e){
        e.preventDefault();
        if($(this).attr('data-stat') != ""){
            $.post( "ajax/get-"+$(this).attr('data-stat')+".php", function( response ) {
                if(response.status == "success"){

                    var chartData = response.data,
                        labelsArray = [],
                        datasetData = [];

                    labelsArray.push('Start');
                    datasetData.push(0);
                    for(var i = 0; i < chartData.length; i++){
                        labelsArray.push(chartData[i].time);
                        datasetData.push(parseInt(chartData[i].amount));
                    }

                    var lineChartData = {
                        labels : labelsArray,
                        datasets : [{
                            label: "",
                            fillColor: "rgba(220,220,220,0.2)",
                            strokeColor: "rgba(220,220,220,1)",
                            pointColor: "rgba(220,220,220,1)",
                            pointStrokeColor: "#fff",
                            pointHighlightFill: "#fff",
                            pointHighlightStroke: "rgba(220,220,220,1)",
                            data : datasetData
                        }]
                    }

                    m59DrawChart('line', lineChartData);

                }
            });
        }
    });

    /**
     *  Display Money Time Weekly
     */

    $("#server-menu").on('click', 'a#moneytimeweekly', function(e){
        e.preventDefault();
        if($(this).attr('data-stat') != ""){
            $.post( "ajax/get-"+$(this).attr('data-stat')+".php", function( response ) {
                if(response.status == "success"){

                    var chartData = response.data,
                        labelsArray = [],
                        datasetData = [];

                    for(var i = 0; i < chartData.length; i++){
                        labelsArray.push(chartData[i].date);
                        datasetData.push(parseInt(chartData[i].amount));
                    }

                    var lineChartData = {
                        labels : labelsArray,
                        datasets : [{
                            label: "",
                            fillColor: "rgba(220,220,220,0.2)",
                            strokeColor: "rgba(220,220,220,1)",
                            pointColor: "rgba(220,220,220,1)",
                            pointStrokeColor: "#fff",
                            pointHighlightFill: "#fff",
                            pointHighlightStroke: "rgba(220,220,220,1)",
                            data : datasetData
                        }]
                    }

                    m59DrawChart('line', lineChartData);

                }
            });
        }
    });

});
