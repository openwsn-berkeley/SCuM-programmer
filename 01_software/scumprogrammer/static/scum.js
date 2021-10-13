var statuspaneinitialized   = false;
var uartpaneinitialized     = false;
var spectrumpaneinitialized = false;

function loadSvg() {
    d3.xml( "/static/scumprogrammer_ui.svg")
       .then(data => {
            // append the SVG to the document
            document.body.append(data.documentElement);
            
            // build the page for the first time
            buildPage();
            
            // load data periodically
            setInterval(function() {
                getData()
            }, 100);
       })
}

function buildPage() {
    console.log('buildPage');
}
    
function getData() {
    console.log('getData');
    $.getJSON( "/data.json", function( data ) {
        updateUI(data);
    });
}

function updateUI(data) {
    console.log('updateUI');
    return; // poipoi
    // (re)position panes
    fullWidth  = $(window).width()-20;
    fullHeight = $(window).height()-90;
    
    d3.select("#statuspane")
       .attr("width",  fullWidth)
       .attr("height", fullHeight*0.50-10)
       .attr("viewBox", '0 0 500 100')
       .attr("preserveAspectRatio", 'none');
    d3.select("#uartpane")
       .style("top",   50+fullHeight*0.50+10)
       .attr("height", fullHeight*0.50)
       .attr("width",  fullWidth*0.40-10)
       .attr("viewBox", '0 0 200 100')
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
    
    if (statuspaneinitialized==false) {
        
        svg
            .append("text")
                .attr("x", 1)
                .attr("y", 6)
                .text('status')
                .attr("class",  "panetitle")
        svg
            .append("text")
                .attr("x",      80)
                .attr("y",      30)
                .attr("class",  "firmwaretext labelcomputer")
        svg
            .append("text")
                .attr("x",      80)
                .attr("y",      40)
                .attr("class",  "firmwaretext labelcomputerinfo")
        svg
            .append("rect")
                .attr("x",      150)
                .attr("y",      60)
                .attr("class",  "button");
        svg
            .append("text")
                .attr("x",      150+1)
                .attr("y",      60+6)
                .text('transfer')
                .attr("class",  "buttontext")
        svg
            .append("text")
                .attr("x",      220)
                .attr("y",      30)
                .attr("class",  "firmwaretext labelgateway")
        svg
            .append("text")
                .attr("x",      220)
                .attr("y",      40)
                .attr("class",  "firmwaretext labelgatewayinfo")
        svg
            .append("rect")
                .attr("x",      300)
                .attr("y",      50)
                .attr("class",  "button");
        svg
            .append("text")
                .attr("x",      300+1)
                .attr("y",      50+6)
                .text('bootload')
                .attr("class",  "buttontext")
        svg
            .append("rect")
                .attr("x",      300)
                .attr("y",      70)
                .attr("class",  "button");
        svg
            .append("text")
                .attr("x",      300+1)
                .attr("y",      70+6)
                .text('reset')
                .attr("class",  "buttontext")
        svg
            .append("text")
                .attr("x",      400)
                .attr("y",      30)
                .attr("class",  "firmwaretext labelscum")
        svg
            .append("text")
                .attr("x",      400)
                .attr("y",      40)
                .attr("class",  "firmwaretext labelscuminfo")
        
        statuspaneinitialized = true;
    }
    
    svg.selectAll(".labelcomputer")
        .text(data.labelcomputer[0])
    svg.selectAll(".labelcomputerinfo")
        .text(data.labelcomputer[1])
    svg.selectAll(".labelgateway")
        .text(data.labelgateway[0])
    svg.selectAll(".labelgatewayinfo")
        .text(data.labelgateway[1])
    svg.selectAll(".labelscum")
        .text(data.labelscum[0])
    svg.selectAll(".labelscuminfo")
        .text(data.labelscum[1])
    
    var chunks  = svg.selectAll(".chunks")
        .data(data.chunks);
    chunks
        .transition()
            .attr("x",      function(d,i) { return 40+150*d+10*(i%8); })
            .attr("y",      function(d,i) { return 50+5*Math.floor(i/8); });
    chunks
        .enter().append("rect")
            .attr("x",      function(d,i) { return 40+150*d+10*(i%8); })
            .attr("y",      function(d,i) { return 50+5*Math.floor(i/8); })
            .attr("width",  4)
            .attr("height", 3)
            .attr("class", "chunks");
}

function updateUartpane(data) {
    svg = d3.select("#uartpane");
    
    if (uartpaneinitialized==false) {
        
        svg
            .append("text")
                .attr("x", 1)
                .attr("y", 6)
                .text('uart')
                .attr("class",  "panetitle")
        svg
            .append("rect")
                .attr("x",      0)
                .attr("y",      90)
                .attr("width",  100)
                .attr("height", 10)
                .attr("class",  "uartchatbg");
        
        uartpaneinitialized = true;
    }
}

function updateSpectrumpane(data) {
    
    svg = d3.select("#spectrumpane");
    
    if (spectrumpaneinitialized==false) {
        
        svg
            .append("text")
                .attr("x", 1)
                .attr("y", 6)
                .text('spectrum')
                .attr("class",  "panetitle")
        
        spectrumpaneinitialized = true;
    }
    
    var rssibars  = svg.selectAll(".rssibars")
        .data(data.rssis);
    rssibars
        .transition()
            .attr("y1", function(d,i) { return 95+d; });
    rssibars
        .enter().append("line")
            .attr("x1", function(d,i) { return 4+3*i; })
            .attr("y1", function(d,i) { return 95+d; })
            .attr("x2", function(d,i) { return 4+3*i; })
            .attr("y2", function(d,i) { return 95; })
            .attr("class", "rssibars");
}
