(function() {
    
    var buildChart = function(data) {
        var getCategories = function() {
            var ret = [];
            _.each(data[0].data, function(item) {
                        for(var i in item) {
                            ret.push(i);
                        }
                        });
            return ret;
        }
        var getSeries = function() {
            var ret = data.slice(0);
            _.each(data, function(item) {
                var dataArry = [];       
                _.each(item.data, function(d) {
                    for (var i in d) {
                        dataArry.push(parseInt(d[i], 10));
                    }
                });
                item.data = dataArry;
             });
            
            return ret;
        }
       var chart = new Highcharts.Chart({
            chart: {
                renderTo: 'chart_wrapper',
                type: 'line',
                marginRight: 130,
                marginBottom: 25
            },
            title: {
                text: 'Monthly Average Temperature',
                x: -20 //center
            },
            subtitle: {
                text: 'Source: WorldClimate.com',
                x: -20
            },
            xAxis: {
                categories: getCategories()
            },
            yAxis: {
                title: {
                    text: 'Temperature (°C)'
                },
                plotLines: [{
                    value: 0,
                    width: 1,
                    color: '#808080'
                }]
            },
            tooltip: {
                formatter: function() {
                        return '<b>'+ this.series.name +'</b><br/>'+
                        this.x +': '+ this.y ;
                }
            },
            legend: {
                layout: 'vertical',
                align: 'right',
                verticalAlign: 'top',
                x: -10,
                y: 100,
                borderWidth: 0
            },
            series: getSeries()
        });
    }

    var buildNavList = function(data) {
        var navList = $('#nav_list');
        _.each(data, function(item, index) {
            var li = $('<li/>').addClass('nav-header')
                               .attr('data-id', item.id)
                               .html(item.name);
            navList.append(li);
            _.each(item.app, function(app, i) {
                var li = $('<li/>').attr({'data-group-id': item.id,
                                          'data-id': app    
                                         });
                if (index === 0 && i === 0) {
                    li.addClass('active');
                }
                var link = $('<a/>').attr('href', '###')
                                    .html(app);
                li.append(link);
                navList.append(li);
            });
        });

        $('#nav_list li:not(.nav-header)').on('click', function() {
            var target = $(this);
            $('#nav_list li').removeClass('active');
            target.addClass('active');
            getReportData();
        });


    }

    
    var getNavData = function(){
        var url = '/api/clusterInfo';
        $.getJSON(url, function(resp) {
                    buildNavList(resp);
                });
    }

    var setTime = function() {
        var today = new Date();

        var endYear = today.getFullYear();
        var endMonth = today.getMonth() + 1;
        var endDate = today.getDate();
        endMonth = ('' + endMonth).length == 1 ? '0' + endMonth : endMonth;
        endDate = ('' + endDate).length == 1 ? '0' + endDate : endDate;

        var start = new Date();
        start.setDate(endDate - 2);

        var startYear = start.getFullYear();
        var startMonth = start.getMonth() + 1;
        var startDate = start.getDate();
        startMonth = ('' + startMonth).length == 1 ? '0' + startMonth : startMonth;
        startDate = ('' + startDate).length == 1 ? '0' + startDate : startDate;

        $('.filter .time-start').val(startYear + '-' + startMonth + '-' + startDate);
        $('.filter .time-end').val(endYear + '-' + endMonth + '-' + endDate);
    }
    
    var getTime = function() {
        var ret = {};
        ret.start = $('.filter .time-start').val();
        ret.end = $('.filter .time-end').val();

        return ret;
    }
    
    
    var getReportData = function() {
        var currentApp = $('#nav_list .active');
        var clusterName = currentApp.attr('data-group-id');
        var appId = currentApp.attr('data-id');
        var time = getTime();
        var url = '/api/base?cluster_name=' + clusterName + '&app_id=' + appId + '&time_start=' + time.start + '&time_end=' + time.end;
        $.getJSON(url, function(data) {
                    buildChart(data);
                 });
    }
    
    $('.apply').on('click', function(){
        getReportData();    
    });

    getNavData();
    setTime();
    getReportData();
    
 })();
