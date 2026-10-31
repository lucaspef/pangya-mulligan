# Avisos legais

*English summary at the end.*

## 1. Natureza do projeto

PangYa Mulligan é um projeto de fãs, sem fins lucrativos, feito para preservação, estudo e continuidade do jogo. Não vende nada, não aceita pagamentos e não opera serviço de jogo.

O projeto não tem afiliação com a Ntreev Soft, a Kakao Games, a Gungho, a Gametree ou qualquer outra desenvolvedora ou publicadora do PangYa, e não foi aprovado nem patrocinado por elas. PangYa e as marcas, nomes e logotipos relacionados pertencem aos seus respectivos titulares e são citados apenas para identificar o programa estudado.

## 2. Licença e ressalva

A licença GNU Affero General Public License v3.0 (arquivo `LICENSE`) cobre **as contribuições próprias do projeto**: o servidor em C#, as alterações e reconstruções escritas pelo projeto, ferramentas e documentação.

**Ressalva.** O cliente em `client/` foi reconstruído a partir do estudo de um programa comercial, e parte do código e dos dados deriva desse programa. Essa parte **mantém os direitos originais dos seus titulares**. A AGPL-3.0 só é concedida sobre o que o projeto tem direito de licenciar; ela não concede nem pretende conceder direitos sobre o jogo original, suas marcas, seu código ou seus dados.

Bibliotecas de terceiros continuam sob suas próprias licenças (seção 5).

## 3. O que o repositório não contém

Nenhum arquivo do jogo: nenhum executável ou DLL original, `.pak`, `.iff`, imagem, modelo, som, música ou vídeo do jogo, nem dados de jogadores. Também não contém SDKs proprietários. Cada pessoa precisa da sua própria cópia do cliente e das dependências descritas em `client/COMPILAR.md`.

O ícone e os bitmaps em `client/src/projectg/` (`ProjectG.ico`, `111.bmp`, `113.bmp`, `123.bmp`) são substitutos neutros. O código mantém, como literais, alguns textos, nomes de arquivo e endereços do programa original, necessários ao seu funcionamento. Os endereços de rede são os do programa original; nenhum é servidor deste projeto.

`server/src/Pangya.Core/Crypto/keytables.bin` e `server/tests/Pangya.Tests/Data/crypto_vectors.txt` contêm tabelas de chave do protocolo do jogo, usadas por compatibilidade.

## 4. Pedidos de remoção

Se você é titular de direitos e quer a retirada de algum conteúdo, abra uma issue em <https://github.com/lucaspef/pangya-mulligan/issues> explicando o pedido. Pedidos legítimos serão atendidos.

## 5. Avisos de terceiros

Vale sempre o aviso presente nos próprios arquivos.

| Componente | Local | Licença |
|---|---|---|
| zlib 1.2.2 | `client/src/fresh/third_party/zlib-1.2.2/` | zlib |
| libpng 1.2.8 | `client/src/fresh/third_party/libpng-1.2.8/` | libpng (`LICENSE`) |
| FreeType 2.2.1 | `client/src/engine/lib/freetype-2.2.1/` | FreeType License (`docs/FTL.TXT`), escolhida entre as opções FTL ou GPLv2. "Portions of this software are copyright © 2006 The FreeType Project (www.freetype.org). All rights reserved." |
| Lua 5.1 | `client/src/shared/LuaSystem/Lua/` | MIT (`COPYRIGHT`) |
| LuaTinker, Kwon-il Lee | `client/src/shared/LuaSystem/lua_tinker/` | Conforme o aviso nos cabeçalhos; o `Licence.txt` original não acompanha a cópia |
| MersenneTwister.h 1.1, Richard J. Wagner; algoritmo de Makoto Matsumoto e Takuji Nishimura | `client/src/shared/mtrand1p1.h` | BSD de 3 cláusulas (no cabeçalho) |
| Mersenne Twister 1997, Makoto Matsumoto e Takuji Nishimura | `client/src/projectg/randomhouse.cpp` | GNU LGPL v2 ou posterior (no cabeçalho) |
| TinyXML, Lee Thomason | `client/src/projectg/tinyxml*`, `client/src/projectg/jp/rftinyxml*`, `client/include/jp/refresh/tinyxml/` | zlib |
| KMlocal 1.7.2, David Mount, com partes da ANN | `client/src/projectg/jp/km*.cpp`, `client/src/projectg/jp/kc*.cpp`, `client/include/jp/kmlocal/` | GNU GPL v2 ou posterior (no cabeçalho), usada aqui sob a versão 3 |
| XUnzip 1.3, Lucian Wischik, com zlib 1.1.3 e minizip 0.15 | `client/src/engine/xunzip.cpp` | Conforme os avisos de zlib e minizip no arquivo |

Dependências externas, instaladas por cada pessoa e não incluídas aqui: Windows SDK e ATL (Microsoft), D3DX 9 (Microsoft), WTL e, em tempo de execução, Miles Sound System e Intel JPEG Library da cópia do jogo. Quem distribuir binários é responsável por verificar a compatibilidade dessas licenças.

---

## English summary

- Non-profit fan project for preservation and study. Not affiliated with, approved or sponsored by Ntreev Soft, Kakao Games, Gungho, Gametree or any PangYa developer or publisher. PangYa and related marks belong to their respective owners.
- The AGPL-3.0 covers **the project's own contributions only**. The client was reconstructed from the study of a commercial program; **code and data derived from it keep their original rights**, and no rights over the original game, its marks, code or data are granted.
- No game files, game assets, proprietary SDKs or player data are included. You need your own copy of the client.
- Rights holders may request removal by opening an issue at <https://github.com/lucaspef/pangya-mulligan/issues>. Legitimate requests will be honored.
- Third-party components keep their own licenses, listed in section 5.
