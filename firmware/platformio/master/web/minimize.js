import { minify } from 'html-minifier';
import fs from 'fs';

function processPage(inputFile, outputFile, macroName, headerGuard) {
    console.log(`Minimizing ${inputFile}`);
    const data = fs.readFileSync(inputFile, 'utf8');
    const result = minify(data, {
        removeAttributeQuotes: true,
        collapseWhitespace: true,
        removeComments: true,
        removeRedundantAttributes: true,
        removeScriptTypeAttributes: true,
        removeStyleLinkTypeAttributes: true,
        removeTagWhitespace: true,
        useShortDoctype: true,
        minifyCSS: true,
        minifyJS: true
    });

    const header =
`#ifndef ${headerGuard}
#define ${headerGuard}
#define ${macroName} "${result.replaceAll(`"`, `\\"`)}"
#endif`

    fs.writeFileSync(outputFile, header);
    console.log(`Generated ${outputFile}`);
}

try {
    processPage('./index.html',  '../include/web_page.h', 'WEB_PAGE', 'WEB_PAGE_H');
    processPage('./update.html', '../include/ota_page.h', 'OTA_PAGE', 'OTA_PAGE_H');
} catch (err) {
    console.error(err);
}
