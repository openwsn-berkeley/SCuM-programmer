<!doctype html>

<meta charset="utf-8">
<title>SCuM Programmer</title>

<link   rel="stylesheet" href="/static/scum.css">
<script src="/static/d3.min.js"></script>
<script src="/static/jquery-3.6.0.min.js" charset="utf-8"></script>
<script src="/static/scum.js" charset="utf-8"></script>

<div id="pagetitle"><strong>SCuM</strong>PROGRAMMER</div>
<svg id="floorplan"></svg>
<div id="versionlabel">version {{version}}</div>

<script id="js">
    $(document).ready(function() {
        gettingThingsInPlace();
        getData();
        // periodically refresh
        setInterval(function() {
            getData()
        }, 100);
    });
</script>