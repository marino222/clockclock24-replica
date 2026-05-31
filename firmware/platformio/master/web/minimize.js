import { minify } from 'html-minifier';
import fs from 'fs';
import path from 'path';

function processPage(inputFile, outputFile, macroName, headerGuard) {
    console.log(`Minimizing ${inputFile}`);
    let data = fs.readFileSync(inputFile, 'utf8');

    // Inject external files referenced as <!-- inject:filename --> comments.
    // Resolved relative to the input file's directory, so each HTML file can
    // have its own companion CSS/JS without hardcoded paths here.
    const dir = path.dirname(path.resolve(inputFile));
    data = data.replace(/<!--\s*inject:([\w.\-/]+)\s*-->/g, (match, filename) => {
        const filePath = path.join(dir, filename.trim());
        if (fs.existsSync(filePath)) {
            console.log(`  Injecting ${filePath}`);
            return fs.readFileSync(filePath, 'utf8');
        }
        console.warn(`  Warning: inject target not found: ${filePath}`);
        return '';
    });

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
