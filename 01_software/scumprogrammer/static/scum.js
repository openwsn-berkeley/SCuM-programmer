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
       .attr("width",  fullWidth*0.50-10)
       .attr("height", fullHeight*0.50)
       .style("top",   50+fullHeight*0.50+10);
    d3.select("#spectrumpane")
       .attr("width",  fullWidth*0.50)
       .attr("height", fullHeight*0.50)
       .style("top",   50+fullHeight*0.50+10)
       .style("left",  10+fullWidth*0.50);
    $("#versionlabel").css({ top: 60+fullHeight+'px' });
    $("#versionlabel").css({ width: fullWidth+'px' });
    $("#urllabel").css({ top: 60+fullHeight+'px' });
    $("#urllabel").css({ width: fullWidth+'px' });
    
    // update statuspane
    // TODO
    
    // update uartpane
    // TODO
    
    // update spectrumpane
    // TODO
    
    // update versionlabel
    $("#versionlabel").html(data.versionlabel);
}
