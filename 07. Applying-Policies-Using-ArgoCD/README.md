# Exercise 7 - Using ArgoCD PolicyGenerator and Kustomize

In this exercise you will go through the Compliance features that come with Open Policy Agent Gatekeeper and the Compliance Operator. You will apply a number of policies to the cluster in order to comply with global security and management standards.

## PolicyGenerator

The PolicyGenerator is a Kustomize plugin to wrap Kubernetes manifests in Policies and also generate PolicySets. Generation can be done locally or through GitOps. We restrict generation sources to local subdirectories for security but starting with RHACM 2.7 the path can point to a Kustomize directory, which allows additional levels of customization as well as supplying a remote URL in the Kustomize file.


<hub> $ cat >> argocd-policy-generator-integration.yaml << EOF
---
apiVersion: argoproj.io/v1alpha1
kind: ArgoCD
metadata:
  name: policies
  namespace: openshift-gitops
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
  name: openshift-gitops-application-controller
  namespace: openshift-gitops
---
apiVersion: argoproj.io/v1alpha1
kind: Application
metadata:
  name: policies-integration
  namespace: openshift-gitops
spec:
  destination:
    namespace: openshift-gitops
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
  namespace: openshift-gitops
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

<hub> $ oc apply -f argocd-policy-generator-integration.yaml
```

Review the results in the UI


 