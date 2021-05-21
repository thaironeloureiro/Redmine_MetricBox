<?php

$validadeEmSegundos = 36000;
//$arquivoCache = '/opt/sei/web/api.redmine/cache/metricas_esp32.html';
$arquivoCache = 'cache/getdadoscfd.html';

$urlDinamica = 'http://localhost/api.redmine/getdadoscfd_dinamico.php';

// Verifica se o arquivo cache existe e se ainda é válido
if (file_exists($arquivoCache) && (filemtime($arquivoCache) > time() - $validadeEmSegundos)) {
    // Lê o arquivo cacheado
    $conteudo = file_get_contents($arquivoCache);
} else {
    // Acessa a versão dinâmica
    //$conteudo = file_get_contents($urlDinamica);
    $conteudo = curl_get_file_contents($urlDinamica);

    // Cria o cache
    file_put_contents($arquivoCache, $conteudo);
}

// Exibe o conteúdo da página
echo $conteudo;


function curl_get_file_contents($URL)
{
        $c = curl_init();
        curl_setopt($c, CURLOPT_RETURNTRANSFER, 1);
        curl_setopt($c, CURLOPT_URL, $URL);
        $contents = curl_exec($c);
        curl_close($c);

        if ($contents) return $contents;
            else return FALSE;
}
?>