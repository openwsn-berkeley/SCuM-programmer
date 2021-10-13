
//=========================== setup ===========================================

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
    
    // buttons can be clicked
    buttonNames = ["bootload","reset","GPIOcal"]
    for (var i in buttonNames) {
        d3.select("#button_"+buttonNames[i]+"_rect")
            .classed("button", true);
    }
    d3.selectAll(".button")
        .on("mouseover", function(e,d) {
            d3.select(this).style("cursor","pointer");
        })
        .on("click", function(e,d) {
            d3.json("/action.json", {
                method:   "POST",
                body:     JSON.stringify({
                    "element": this.id
                }),
                headers:  {
                    "Content-type": "application/json; charset=UTF-8"
                }
            });
        })
}

//=========================== periodic ========================================

function getData() {
    
    $.getJSON( "/data.json", function( data ) {
        updateUI(data);
    });
}

function updateUI(data) {
    
    // cellText
    for (const [k,v] of Object.entries(data['cellText'])) {
        setCellText(k,v)
    }
    
    // isconnected
    // TODO only on change
    d3.select("#usb_pc")
        .transition()
        .duration(300)
        .style('stroke',data['isconnected']? 'green': 'grey')
    
    d3.select("#button_GPIOcal_rect")
        .transition()
        .duration(300)
        .style('fill',data['GPIOcalIsOn']? 'grey': 'blue')
}

function updateSpectrumpane(data) {
    
    svg = d3.select("#spectrum_pane");
    
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

//=========================== helpers =========================================

function setCellText(cellName,cellText) {
    
    var svg = d3.select('#pane_dynamic')
    
    data    = [cellText]
    
    x       =   parseFloat(d3.select("#"+cellName).attr("x"))
              + parseFloat(d3.select("#"+cellName).attr("width"))/2;
    y       =   parseFloat(d3.select("#"+cellName).attr("y"))
              + parseFloat(d3.select("#"+cellName).attr("height"))/2+7;
              
    var texboxes = svg.selectAll("."+cellName+"-textbox")
        .data(data)
    texboxes
        .enter().append("text")
            .attr("x", x)
            .attr("y", y)
            .attr("class", cellName+"-textbox cellText")
            .attr("x", x)
        .merge(texboxes)
            .text(function(d) { return d;})
}