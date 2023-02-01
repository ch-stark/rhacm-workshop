apiVersion: v1
data:
  custom_rules.yaml: |
    groups:
      - name: cluster-health
        rules:
        - alert: ClusterMemoryHighUsage
          annotations:
            summary: Notify when memory utilization on a cluster is greater than the defined utilization limit - 20%
            description: "The cluster has a high memory usage: {{ $value }} for {{ $labels.cluster }}."
          expr: |
            1 - sum(:node_memory_MemAvailable_bytes:sum) by (cluster) / sum(kube_node_status_allocatable{resource="memory"}) by (cluster) > 0.2
          for: 5s
          labels:
            cluster: "{{ $labels.cluster }}"
            severity: critical
kind: ConfigMap
metadata:
  name: thanos-ruler-custom-rules
  namespace: open-cluster-management-observability
