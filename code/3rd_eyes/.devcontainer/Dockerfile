ARG DOCKER_TAG=v6.1
FROM espressif/idf:${DOCKER_TAG}

ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8
ENV IDF_CCACHE_ENABLE=1

RUN apt-get update -y \
    && apt-get install -y --no-install-recommends udev \
    && rm -rf /var/lib/apt/lists/*

# Install ESP-IDF MCP feature
RUN /bin/bash -lc '\
    python3 /opt/esp/idf/tools/idf_tools.py install-python-env --features=mcp \
    && source /opt/esp/idf/export.sh >/dev/null 2>&1 \
    && python -m pip install --upgrade "mcp[cli]>=1.29,<2" \
    '

RUN echo "source /opt/esp/idf/export.sh > /dev/null 2>&1" >> ~/.bashrc

ENTRYPOINT ["/opt/esp/entrypoint.sh"]

CMD ["/bin/bash", "-c"]