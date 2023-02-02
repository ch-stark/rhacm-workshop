# Exercise 4 - Application Lifecycle

# ArgoCD Integration

This section discusses the process of deploying an application using ArgoCD in an Advanced Cluster Management for Kubernetes environment. The section will follow the process of ArgoCD installation, integration with RHACM and application deployment.

Before you begin, make sure to delete all of the resources you created in the previous exercise -

```
<hub> $ oc delete project webserver-acm
```

## ArgoCD Installation

As described in the workshop. An ArgoCD / OpenShift GitOps instance has to be installed in order to begin the integration with RHACM. Install the `openshift-gitops` operator by applying the next resource to the hub cluster -

```
<hub> $ cat >> openshift-gitops-operator.yaml << EOF
---
apiVersion: operators.coreos.com/v1alpha1
kind: Subscription
metadata:
  name: openshift-gitops-operator
  namespace: openshift-operators
spec:
  channel: stable
  installPlanApproval: Automatic
  name: openshift-gitops-operator
  source: redhat-operators
  sourceNamespace: openshift-marketplace
EOF

<hub> $ oc apply -f openshift-gitops-operator.yaml
```

After installing the operator on the hub cluster. Create the ArgoCD CustomResource. The ArgoCD CR spins an instance of ArgoCD using the `openshift-gitops` operator.

```
<hub> $ cat >> argocd.yaml << EOF
---
apiVersion: argoproj.io/v1alpha1
kind: ArgoCD
metadata:
  finalizers:
    - argoproj.io/finalizer
  name: openshift-gitops
  namespace: openshift-gitops
spec:
  server:
    autoscale:
      enabled: false
    grpc:
      ingress:
        enabled: false
    ingress:
      enabled: false
    resources:
      limits:
        cpu: 500m
        memory: 256Mi
      requests:
        cpu: 125m
        memory: 128Mi
    route:
      enabled: true
    service:
      type: ''
  grafana:
    enabled: false
EOF

<hub> $ oc apply -f argocd.yaml
```

Make sure that the ArgoCD instance is running by navigating to ArgoCD's web UI. The URL can be found be running the next command -

```
<hub> $ oc get route -n openshift-gitops
NAME                      HOST/PORT                                  PATH   SERVICES                  PORT    TERMINATION            WILDCARD
...
openshift-gitops-server   openshift-gitops-server-openshift-gitops.<FQDN>   openshift-gitops-server   https   passthrough/Redirect   None
```

Log into the ArgoCD instance by pressing on `Log In via OpenShift`.

Now that you have a running instance of ArgoCD, let's integrate it with RHACM!

## Preparing RHACM for ArgoCD Integration

In this part you will create the resources to import `local-cluster` into ArgoCD's managed clusters.

Create the next ManagedClusterSet resource. The ManagedClusterSet resource will include the `local-cluster` cluster. The ManagedClusterSet resource is associated with the `openshift-gitops` namespace.

```
<hub> $ cat >> managedclusterset.yaml << EOF
---
apiVersion: cluster.open-cluster-management.io/v1beta1
kind: ManagedClusterSet
metadata:
  name: all-clusters
EOF

<hub> $ oc apply -f managedclusterset.yaml
```

Now, import `local-cluster` into the ManagedClusterSet resource. Importation will be done by adding the `cluster.open-cluster-management.io/clusterset: all-clusters` label to the `local-cluster` ManagedCluster resource -

```
<hub> $ oc edit managedcluster local-cluster
...
labels:
...
    cloud: Amazon
    cluster.open-cluster-management.io/clusterset: all-clusters
...
```

Create the ManagedClusterSetBinding resource to bind the `local-cluster` ManagedClusterSet resource to the `openshift-gitops` resource. Creating the ManagedClusterSetBinding resource will allow ArgoCD to access `local-cluster` information and import it into its management stack.

```

 $ cat >> managedclustersetbinding.yaml << EOF
---
apiVersion: cluster.open-cluster-management.io/v1beta1
kind: ManagedClusterSetBinding
metadata:
  name: global
  namespace: openshift-gitops
spec:
  clusterSet: global
EOF

<hub> $ oc apply -f managedclustersetbinding.yaml
```

Create the Placement resource and bind it to `all-clusters` ManagedClusterSet. Note that you will not be using any special filters in this exercise.

```
<hub> $ cat >> placement.yaml << EOF
---
apiVersion: cluster.open-cluster-management.io/v1beta1
kind: Placement
metadata:
  name: global
  namespace: openshift-gitops
spec:
  clusterSets:
    - global
EOF

<hub> $ oc apply -f placement.yaml
```

Create the GitOpsServer resource to indicate the location of ArgoCD and the placement resource -

```
<hub> $ cat >> gitopsserver.yaml << EOF
---
apiVersion: apps.open-cluster-management.io/v1beta1
kind: GitOpsCluster
metadata:
  name: gitops-cluster
  namespace: openshift-gitops
spec:
  argoServer:
    cluster: local-cluster
    argoNamespace: openshift-gitops
  placementRef:
    kind: Placement
    apiVersion: cluster.open-cluster-management.io/v1alpha1
    name: global
EOF

<hub> $ oc apply -f gitopsserver.yaml
```

Make sure that `local cluster` is imported into ArgoCD. In ArgoCD's web UI, on the left menu bar, navigate to **Manage your repositories, projects, settings** -> **Clusters**. You should see `local-cluster` in the cluster list.

![argocd-cluster](images/argocd-cluster.png)

## Deploying an ApplicationSet using ArgoCD

Now that you integrated ArgoCD with RHACM, let's deploy an ApplicationSet resource using ArgoCD. The applications you're going to create in this part are based on the same applications you have created in the beginning of the exercise - One web server application for a development environment and one for a production environment.

The applications are based on one [helm](https://helm.sh/) chart. Each application in the set is identified by its own unique `values.yaml` file. The applications are using the same baseline kubernetes resources at - [exercise-argocd/application-resources/templates](exercise-argocd/application-resources/templates), but they are using different `values` files at - [exercise-argocd/application-resources/values](exercise-argocd/application-resources/values). Each instance of the application uses a separate values set. The ApplicationSet resource iterates over the directories in the [exercise-argocd/application-resources/values](exercise-argocd/application-resources/values) directory and creates an instance of an application for each directory name.

To create the ApplicationSet resource run the next commands -

```
<hub> $ oc apply -f https://raw.githubusercontent.com/ch-stark/rhacm-workshop/master/04.Application-Lifecycle/exercise-argocd/argocd-resources/appproject.yaml

<hub> $ oc apply -f https://raw.githubusercontent.com/ch-stark/rhacm-workshop/master/04.Application-Lifecycle/exercise-argocd/argocd-resources/applicationset.yaml
```

Note that two application instances have been created in the ArgoCD UI -

![argocd-applications](images/argocd-applications.png)

After viewing the applications and their resources in the ArgoCD dashboard, log into RHACM's web console, and navigate to **Applications**. Note that RHACM identifies the deployed ApplicationSet and provides an entry for both applications -

![argocd-rhacm-applications](images/argocd-rhacm-applications.png)

The deployed application resources can be seen in the ApplicationSet instance in RHACM -

![rhacm-argocd-app-details](images/rhacm-argocd-app-details.png)

Make sure that the application is available by navigating to its Route resource.

```
<hub> $ oc get route -n webserver-prod

NAME        HOST/PORT                              PATH                SERVICES    PORT       TERMINATION   WILDCARD
webserver   webserver-webserver-prod.apps.<FQDN>   /application.html    webserver   8080-tcp   edge          None
```
