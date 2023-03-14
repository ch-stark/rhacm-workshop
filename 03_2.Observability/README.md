# Exercise 3 - Observability

In this exercise you enable and use the `Observability` function in Red Hat Advanced Cluster Management. You will configure observability, explore the built-in dashboards, enable custom alerts using Thanos Ruler and design custom dashboards for your own organizational needs.

### 3.5 - Deploy ArgoCD Dashboard
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

On managed cluster, install the Configmap for ACM observability allowlist, so that the opa metrics whose name is “opa_scorecard_violations” will be collected by acm observability addon.

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



 
