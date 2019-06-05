let createError = require('http-errors');
let express = require('express');
let path = require('path');
//let cookieParser = require('cookie-parser');
let logger = require('morgan');
const http = require('http');

let app = express();

app.use(logger('dev'));
app.use(express.json());
app.use(express.urlencoded({ extended: false }));
//app.use(cookieParser());

app.set('view engine', 'pug');

var mathjaxRouter = require('./mathjax');

app.use('/mathjax', mathjaxRouter);

// catch 404 and forward to error handler
app.use(function(req, res, next) {
  next(createError(404));
});

// error handler
app.use(function(err, req, res, next) {
  // set locals, only providing error in development
  res.locals.message = err.message;
  res.locals.error = req.app.get('env') === 'development' ? err : {};

  // render the error page
  res.status(err.status || 500);
  //res.render('error');
});


const hostname = '127.0.0.1';
const port = 3434;

app.set('port', port);

const server = http.createServer(app);

server.listen(port, hostname, () => {
  console.log(`Server running at http://${hostname}:${port}/`);
});
