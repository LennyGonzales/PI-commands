## Server
systemctl daemon-reload

systemctl enable ifnetshow.service

systemctl start ifnetshow.service

systemctl status ifnetshow.service

## Client
Dépannage
Si le service ne démarre pas :
Consultez les journaux pour des informations détaillées :
    journalctl -u ifnetshow.service
Assurez-vous que le pare-feu autorise le port 12345 sur le serveur :
    sudo ufw allow 12345/tcp