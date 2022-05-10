const path = require('path');

module.exports = {
    mode: "development",

    devServer: {
        https: true,
        headers: { "Access-Control-Allow-Origin": "*",
                  // "Origin-Agent-Cluster": "?0"
                },
        host: '0.0.0.0',
        
        static: {
            directory: path.join(__dirname, 'public'),
        },
        allowedHosts: 'all',

        compress: true,
        port: 8000,
    },
};