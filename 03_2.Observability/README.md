# Exercise 3 - Observability

In this exercise you enable and use the `Observability` function in Red Hat Advanced Cluster Management. You will configure observability, explore the built-in dashboards, enable custom alerts using Thanos Ruler and design custom dashboards for your own organizational needs.

### 3.5 - Deploy ArgoCD Dashboard

Create observability-metrics-custom-allowlist.yaml

```
cat >observability-metrics-custom-allowlist.yaml<<YAML
kind: ConfigMap
apiVersion: v1
metadata:
  name: observability-metrics-custom-allowlist
data:
  metrics_list.yaml: |
    names:
      - argocd_cluster_info
      - argocd-server-metrics
      - argocd_app_info
      - argocd_app_sync_total
      - argocd_app_reconcile_count
      - argocd_app_reconcile_bucket
      - argocd_app_k8s_request_total
      - argocd_kubectl_exec_pending
      - argocd-metrics
      - argocd_cluster_api_resource_objects
      - argocd_cluster_api_resources
      - argocd_git_request_total
      - argocd_git_request_duration_seconds_bucket
      - argocd-repo-server
      - argocd_redis_request_total
YAML
```
Apply config map against RHACM
```
oc apply -n open-cluster-management-observability -f observability-metrics-custom-allowlist.yaml
```
Dashboard List

All the command beow need to be ran against the RHACM cluster in order to be shown on the grafana dashboard.

Load argocd dashboard
```
oc create -f config-files/argocd-dashboard.yaml
```


### 3.6 - Deploy OPA Dashboard

Collect Metrics/Create Dashboard for Open Policy Agent

Preq:
* ACM 2.7 or higher with observability installed
* User-Defined Projects Monitoring is enabled in managed cluster

### Install OPA related resources.

In the following we have gatekeeper installed

On your managed cluster, complete the steps 4 in https://github.com/mcelep/opa-scorecard#demo.
After those steps, the OPA components and some demo constraints already created. 

Also, an Prometheus exporter is created and it will provide the endpoint to provide the policy violation metrics.

## Create/Install ACM observability resources

On managed cluster, install ServiceMonitor, so that the opa metrics can be collected by OpenShift User Workload Prometheus.
After this step, you should see the opa metrics available in OpenShift metrics page by search {job=”opa-exporter”}

On managed cluster, install the Configmap for [ACM observability allowlist](https://raw.githubusercontent.com/marcolan018/opa-scorecard/acm/acm-resources/allowlist.yaml), so that the opa metrics whose name is “opa_scorecard_violations” will be collected by acm observability addon.

After this step, you should find the metrics “opa_scorecard_violations” available in ACM Grafana console -> Explore in Hub side

Based on the original dashboard json, create the Configmap for opa dashboard and install it ).
Just create a yaml like below, and copy/paste the original dashboard json’s content under [opa-violations.json](https://raw.githubusercontent.com/marcolan018/opa-scorecard/acm/acm-resources/dashboard.yaml) in the yaml.

```
apiVersion: v1
data:
 opa-violations.json: |-
   {}
kind: ConfigMap
metadata:
 name: opa-violations
 namespace: open-cluster-management-observability
 labels:
   general-folder: 'true'
   grafana-custom-dashboard: 'true'
```



 
