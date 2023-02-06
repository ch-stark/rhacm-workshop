# Exercise 7 - Ansible-AutomationPlatform-Integration

In this exercise you will go through Ansible-AutomationPlatform-Integration with Red Hat Advanced Cluster Management for Kubernetes. You will associate AnsibleJob hooks to applications and integrate AnsibleJobs with policy violations. Ansible Automation Platform has already been configured for your use by the instructor. You will only configure Red Hat Advanced Cluster Management for Kubernetes.

The instructor will provide you with -

* Ansible Automation Platform URL
* Ansible Automation Platform web UI username / password
* Ansible Automation Platform Access Token for API requests

## Before You Begin

In this section you will create the basic integration between RHACM and Ansible-Automation-Platform-. The integration is based on `Ansible Automation Platform Operator`. Make sure to install the operator before you begin the next exercises.

Installing the operator can be done by running the next commands on the hub cluster -

```
<hub> $ oc create namespace ansible-automation-platform

<hub> $ cat >> ansible-operator.yaml << EOF
apiVersion: operators.coreos.com/v1
kind: OperatorGroup
metadata:
  name: ansible-automation-operator-gp
  namespace: ansible-automation-platform
---
apiVersion: operators.coreos.com/v1alpha1
kind: Subscription
metadata:
  name: ansible-automation-operator
  namespace: ansible-automation-platform
spec:
  channel: stable-2.3-cluster-scoped
  installPlanApproval: Automatic
  name: ansible-automation-platform-operator
  source: redhat-operators
  sourceNamespace: openshift-marketplace
EOF

<hub> $ oc apply -f ansible-operator.yaml
```

The operator will now begin the installation process.


Setup the controller

<hub> $ cat >> ansible-controller.yaml << EOF
apiVersion: automationcontroller.ansible.com/v1beta1
kind: AutomationController
metadata:
  name: controller
  namespace: ansible-automation-platform
spec:
  create_preload_data: true
  route_tls_termination_mechanism: Edge
  garbage_collect_secrets: false
  ingress_type: Route
  loadbalancer_port: 80
  no_log: true
  image_pull_policy: IfNotPresent
  projects_storage_size: 8Gi
  auto_upgrade: true
  task_privileged: false
  projects_storage_access_mode: ReadWriteMany
  set_self_labels: true
  projects_persistence: false
  replicas: 1
  admin_user: admin
  loadbalancer_protocol: http
  nodeport_port: 30080
EOF

<hub> $ oc apply -f ansible-controller.yaml
```

```

## Ansible Automation Platform Governance Integration

In this section, you will configure Ansible Automation Platform Jobs to run as a violation is initiated in one of your policies.

### Setting up Authentication using the CLI


<hub> $ cat >> ansible-policywithautomation.yaml << EOF
---
apiVersion: v1
kind: Secret
type: Opaque
metadata:
  name: ansible-credential-auto
  namespace: policies
  labels:
    cluster.open-cluster-management.io/credentials: ""
    cluster.open-cluster-management.io/type: "ans"
stringData:
  host: "https://controller-ansible-automation-platform.apps.policy-xxxx"
  token: "xxx"
---
apiVersion: policy.open-cluster-management.io/v1
kind: Policy
metadata:
  name: policy-test-ansible-integration-auto
  namespace: policies
  annotations:
    policy.open-cluster-management.io/categories: CM Configuration Management
    policy.open-cluster-management.io/controls: CM-2 Baseline Configuration
    policy.open-cluster-management.io/standards: NIST SP 800-53
spec:
  disabled: false
  policy-templates:
    - objectDefinition:
        apiVersion: policy.open-cluster-management.io/v1
        kind: ConfigurationPolicy
        metadata:
          name: policy-namespace-auto
        spec:
          namespaceSelector:
            exclude:
              - kube-*
            include:
              - default
          object-templates:
            - complianceType: musthave
              objectDefinition:
                apiVersion: v1
                kind: Namespace
                metadata:
                  name: notexist
          pruneObjectBehavior: None
          remediationAction: inform
          severity: low
  remediationAction: inform
---
apiVersion: apps.open-cluster-management.io/v1
kind: PlacementRule
metadata:
  name: policy-test-ansible-integration-placement-auto
  namespace: policies
spec:
  clusterConditions: []
  clusterSelector:
    matchExpressions:
      - key: name
        operator: In
        values:
          - local-cluster
---
apiVersion: policy.open-cluster-management.io/v1
kind: PlacementBinding
metadata:
  name: policy-test-ansible-integration-placement-auto
  namespace: policies
placementRef:
  name: policy-test-ansible-integration-placement-auto
  apiGroup: apps.open-cluster-management.io
  kind: PlacementRule
subjects:
  - name: policy-test-ansible-integration-auto
    apiGroup: policy.open-cluster-management.io
    kind: Policy
---
apiVersion: policy.open-cluster-management.io/v1beta1
kind: PolicyAutomation
metadata:
  name: policy-test-ansible-integration-policy-automation-auto
  namespace: policies
spec:
  automationDef:
    name: Demo Job Template
    extra_vars:
      urgent: yes
    policyViolationsLimit: 10
    secret: ansible-credential-auto
    type: AnsibleJob
  delayAfterRunSeconds: 60
  mode: everyEvent
  policyRef: policy-test-ansible-integration-auto
EOF

<hub> $ oc apply -f ansible-policywithautomation.yaml