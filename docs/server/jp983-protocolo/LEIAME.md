# Protocolo do cliente JP 983 — material da análise do binário (jp983src)

Copiado de out/V4 e V5 (frente C, 11/10/2026). Tudo tirado do exe JP.R7.983.00 (dump de 09/11/2017).

- V4_RELATORIO.md — resumo e método (ids, assinaturas de pacote, tabelas .iff).
- ids_servidor.tsv — ids dos pacotes servidor → cliente: KR 645 × JP 983 (deslocamentos por faixa).
- ids_cliente.tsv — ids dos pacotes cliente → servidor: KR 645 × JP 983.
- pacotes_envio_983.tsv — 624 lugares do 983 que montam pacote (261 ids), com a sequência de escritas
  (E1/E2/E4/E8/ES/EX/EB(n) = byte/word/dword/qword/string/.../bloco de n bytes).
- pacotes_recebe_983.tsv — 491 casos tratados no 983 (despachante, id, leituras D1/D2/D4/...).
- rx_cmp.tsv / pk_cmp.tsv — comparação com o código do 645 (o que mudou além do id).
- iff_983.tsv — tamanho de registro das 71 tabelas .iff que o 983 lê.
- V5/ — rede do 983 (endereços, portas, URLs, constantes) e o que o rugburn do usuário altera.
