#!/bin/sh

./.installreq.sh
./.buildurho.sh

git pull
qmake heXon.pro
sudo make install
update-icon-caches ~/.local/share/icons/
