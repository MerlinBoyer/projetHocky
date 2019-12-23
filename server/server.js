const express   = require('express');
const server    = express();
const ejs       = require('ejs');

server.set('view engine', 'ejs');


server.get('/', function (req, res, next) {
	res.render('../pages/index.ejs')
})



/*
*   Launch
*/
server.listen(process.env.PORT || 8080, function () {
    console.log('Your node js server is running on port', process.env.PORT || 8080);
});