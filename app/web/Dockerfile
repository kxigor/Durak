FROM docker.io/nginxinc/nginx-unprivileged:stable-alpine
COPY nginx.conf /etc/nginx/nginx.conf
COPY index.html style.css game.js rooms.js favicon.svg /usr/share/nginx/html/
USER 101:101
ENTRYPOINT ["nginx"]
CMD ["-g","daemon off;"]
