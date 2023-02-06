# Exercise 7 - Using ArgoCD PolicyGenerator and Kustomize

In this exercise you will apply policies using ArgoCD

## PolicyGenerator

The PolicyGenerator is a Kustomize plugin to wrap Kubernetes manifests in Policies and also generate PolicySets. Generation can be done locally or through GitOps. We restrict generation sources to local subdirectories for security but starting with RHACM 2.7 the path can point to a Kustomize directory, which allows additional levels of customization as well as supplying a remote URL in the Kustomize file.


<hub> $ cat >> argocd-policy-generator-integration.yaml << EOF
---
apiVersion: cluster.open-cluster-management.io/v1beta2
kind: ManagedClusterSetBinding
metadata:
  name: global
  namespace: policies
spec:
  clusterSet: global
---
apiVersion: v1
kind: Namespace
metadata:
  name: openshift-gitops-operator
---
apiVersion: v1
kind: Namespace
metadata:
  name: policies
---
apiVersion: operators.coreos.com/v1
kind: OperatorGroup
metadata:
  name: openshift-gitops-group
  namespace: openshift-gitops-operator
spec:
  targetNamespaces: []
---
apiVersion: operators.coreos.com/v1alpha1
kind: Subscription
metadata:
  name: openshift-gitops-operator
  namespace: openshift-gitops-operator
spec:
  channel: "gitops-1.7"
  installPlanApproval: Automatic
  name: openshift-gitops-operator
  source: redhat-operators
  sourceNamespace: openshift-marketplace
  config:
    env:
      - name: ARGOCD_CLUSTER_CONFIG_NAMESPACES
        value: 'openshift-gitops, policies'
EOF

<hub> $ oc apply -f argocd-policy-generator-integration.yaml
```


wait for ca 30 seconds to complete the installation of the Operator




<hub> $ cat >> argocd-policy-generator-integration-config.yaml << EOF
---
apiVersion: argoproj.io/v1alpha1
kind: ArgoCD
metadata:
  name: policies
  namespace: policies
spec:
  resourceTrackingMethod: label+annotation 
  server:
    resources:
      limits:
        cpu: 500m
        memory: 256Mi
      requests:
        cpu: 125m
        memory: 128Mi
    route:
      enabled: true
  repo:
    resources:
      limits:
        cpu: '1'
        memory: 512Mi
      requests:
        cpu: 250m
        memory: 256Mi
    env:
    - name: KUSTOMIZE_PLUGIN_HOME
      value: /etc/kustomize/plugin
    initContainers:
    - args:
      - cp /etc/kustomize/plugin/policy.open-cluster-management.io/v1/policygenerator/PolicyGenerator
        /policy-generator/PolicyGenerator
      command:
      - sh
      - -c
      image: registry.redhat.io/rhacm2/multicluster-operators-subscription-rhel8:v2.6.2
      name: policy-generator-install
      volumeMounts:
      - mountPath: /policy-generator
        name: policy-generator
    volumeMounts:
    - mountPath: /etc/kustomize/plugin/policy.open-cluster-management.io/v1/policygenerator
      name: policy-generator
    volumes:
    - emptyDir: {}
      name: policy-generator
  kustomizeBuildOptions: --enable-alpha-plugins
  dex:
    openShiftOAuth: true
    resources:
      limits:
        cpu: 500m
        memory: 256Mi
      requests:
        cpu: 250m
        memory: 128Mi
  rbac:
    defaultPolicy: role:readonly
    policy: g, system:cluster-admins, role:admin
    scopes: '[groups]'
  redis:
    resources:
      limits:
        cpu: 500m
        memory: 256Mi
      requests:
        cpu: 250m
        memory: 128Mi
  controller:
    resources:
      limits:
        cpu: '2'
        memory: 2Gi
      requests:
        cpu: 250m
        memory: 1Gi
---
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRoleBinding
metadata:
  name: cluster-manager-admin-to-argocd
roleRef:
  apiGroup: rbac.authorization.k8s.io
  kind: ClusterRole
  name: cluster-admin
subjects:
- kind: ServiceAccount
  name: policies-argocd-application-controller
  namespace: policies
---
apiVersion: argoproj.io/v1alpha1
kind: Application
metadata:
  name: policies_gatekeeper
  namespace: policies
spec:
  destination:
    namespace: policies
    server: https://kubernetes.default.svc
  project: policies
  source:
    path: .
    repoURL: https://github.com/ch-stark/gatekeeper-examples
    targetRevision: HEAD
  syncPolicy:
    syncOptions:
    - CreateNamespace=true
    automated:
      selfHeal: false
      prune: true           
---
apiVersion: argoproj.io/v1alpha1
kind: AppProject
metadata:
  name: policies
  namespace: policies
  # Finalizer that ensures that project is not deleted until it is not referenced by any application
  finalizers:
    - resources-finalizer.argocd.argoproj.io
spec:
  sourceRepos:
  - '*'
  destinations:
  - namespace: '*'
    server: '*'
  clusterResourceWhitelist:
  - group: '*'
    kind: '*'    
EOF

<hub> $ oc apply -f argocd-policy-generator-integration-config.yaml
```





Review the results in the UI


 