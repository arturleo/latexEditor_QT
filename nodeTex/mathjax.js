var express = require('express');
var router = express.Router();

// a simple TeX-input example
let mjAPI = require("mathjax-node");
let jsdom = require("jsdom").jsdom;
mjAPI.config({
  MathJax: {
    showProcessingMessages: true,
    jax: ["input/TeX", "output/svg"],
    TeX: {}
	}
});

router.use(express.json());
router.post('/', function(req, res){
  console.log(req.body.data);      // your JSON
  //res.send(req.body);    // echo the result back
  
  let ans=req.body.data;
  mjAPI.start();
  mjAPI.typeset({
    math: ans,
    format: "TeX", // or "inline-TeX", "MathML"
    svg:true,      // or svg:true, or html:true
  }, function (data) {
    if (!data.errors) {console.log(data)}
	  console.log(data);
	  res.setHeader('Content-Type', 'image/svg+xml');
	  //res.sendFile(data.svg);
	  res.send(data);
  });
  
  //console.log("\ncheck\n");
  //res.render("<img src=data>svg</img>");
  return;
});
  
router.get('/',function(req, res, next) {
  //console.log("get"+req.query.data+"\n");
  let ans=req.query.data;
  //console.log("ans"+ans+"\n");
  //let math = jsdom(ans);
  //console.log("ok1\n");
  
  mjAPI.start();
  //console.log("ok start\n");
  mjAPI.typeset({
    math: ans,
    format: "TeX", // or "inline-TeX", "MathML"
    svg:true,      // or svg:true, or html:true
  }, function (data) {
    if (!data.errors) {console.log(data)}
	//console.log(data);
	  res.setHeader('Content-Type', 'image/svg+xml');
	  //res.sendFile(data.svg);
	  res.send(data.svg);
  });
  //console.log("\ncheck\n");
  //res.render("<img src=data>svg</img>");
  return;
});

module.exports = router;