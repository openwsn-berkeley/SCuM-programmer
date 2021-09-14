var statuspaneinitialized   = false;
var uartpaneinitialized     = false;
var spectrumpaneinitialized = false;

function getData() {
    $.getJSON( "/data.json", function( data ) {
        updateUI(data);
    });
}

function updateUI(data) {
    
    // (re)position panes
    fullWidth  = $(window).width()-20;
    fullHeight = $(window).height()-90;
    
    d3.select("#statuspane")
       .attr("width",  fullWidth)
       .attr("height", fullHeight*0.50-10);
    d3.select("#uartpane")
       .style("top",   50+fullHeight*0.50+10)
       .attr("height", fullHeight*0.50)
       .attr("width",  fullWidth*0.40-10)
       .attr("viewBox", '0 0 100 100')
       .attr("preserveAspectRatio", 'none');
    d3.select("#spectrumpane")
       .attr("width",  fullWidth*0.60)
       .attr("height", fullHeight*0.50)
       .style("left",  10+fullWidth*0.40)
       .style("top",   50+fullHeight*0.50+10)
       .attr("viewBox", '0 0 246 100')
       .attr("preserveAspectRatio", 'none');
    $("#versionlabel").css({ top: 60+fullHeight+'px' });
    $("#versionlabel").css({ width: fullWidth+'px' });
    $("#urllabel").css({ top: 60+fullHeight+'px' });
    $("#urllabel").css({ width: fullWidth+'px' });
    
    // update statuspane
    updateStatuspane(data.statuspane);
    
    // update uartpane
    updateUartpane(data.uartpane);
    
    // update spectrumpane
    updateSpectrumpane(data.spectrumpane);
    
    // update versionlabel
    $("#versionlabel").html(data.versionlabel);
}

function updateStatuspane(data) {
    svg = d3.select("#statuspane");
}

function updateUartpane(data) {
    svg = d3.select("#uartpane");
    
    if (statuspaneinitialized==false) {
        
        svg
            .append("rect")
                .attr("x",      0)
                .attr("y",      90)
                .attr("width",  100)
                .attr("height", 10)
                .attr("class",  "uartchatbg");
        
        statuspaneinitialized = true;
    }
}

function updateSpectrumpane(data) {
    
    svg = d3.select("#spectrumpane");
    
    var rssibars  = svg.selectAll(".rssibars")
        .data(data.rssis);
    rssibars
        .transition()
            .attr("x1", function(d,i) { return 4+3*i; })
            .attr("y1", function(d,i) { return 95+d; })
            .attr("x2", function(d,i) { return 4+3*i; })
            .attr("y2", function(d,i) { return 95; });
    rssibars
        .enter().append("line")
            .attr("x1", function(d,i) { return 4+3*i; })
            .attr("y1", function(d,i) { return 95+d; })
            .attr("x2", function(d,i) { return 4+3*i; })
            .attr("y2", function(d,i) { return 95; })
            .attr("class", "rssibars");
}
