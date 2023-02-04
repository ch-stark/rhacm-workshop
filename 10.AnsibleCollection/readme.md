
Ansible Galaxy Collection: https://galaxy.ansible.com/stolostron/core
Demo repo: https://github.com/stolostron/acm-ansible-collection-demo


https://github.com/stolostron/acm-ansible-collection-demo#set-up-and-run-the-demo-from-ansible-automation-controller


# Get ready with Kubernetes MultiCluster Management using Ansible Automation Platform

Authors :  Christian Stark


## Introduction

Whether one cluster or a thousand, operating and automating a Kubernetes fleet can be rife with challenges. Ansible is a great toolbox to help overcome these challenges, but translating your playbooks to a fleet of clusters can be a headache! The stolostron.core Ansible Collection supercharges your playbooks with easy, secure, and ansible-native access across your Kubernetes fleet powered by the robust multicluster management layer provided by Red Hat MultiCluster Engine and Advanced Cluster Management for Kubernetes. Stolostron.core allows you to leverage the power of Ansible and its Galaxy of Collections against every Kubernetes cluster in your fleet by auto-populating the Ansible Inventory with your clusters, allowing you to target jobs against specific clusters or sets of clusters. Stolostron.core handles the connectivity, access, allowing you to focus on the business logic for your playbook and let the collection handle the rest!

If you are already working with Ansible Automation Platform (AAP) and you are either already using the [Ansible Kubernetes-Collection](https://github.com/ansible-collections/community.kubernetes) or you like to get started with OpenShift/Kubernetes then we now offer an Ansible-Collection which helps you to easily get started to Managed RedHat Advanced Cluster Management for Kubernetes (RHACM). In ACM 2.5 we release this as Dev-Preview and are working hard to make this Tech-Preview and GA in further releases.

You can control permissions and you automatically know about the state of each Cluster of your fleet as we maintain a dynamic inventory.

This will enable you to only focus entirely on the Business Logic.

In the following we are going to explain step by step how you can securely do any Kubernetes related Operation against any Cluster in the fleet which RHACM managed.

Kubernetes allows users to declaratively manage the desired state of the system. But much like any other declarative state management system, sometimes imperative and idempotent is needed to amplify its power and provide ordering and provide powerful templitization functionality.

The tools in the collection allows users of ACM to be able to connect with managed kubernetes clusters and do ANYTHING on the managed cluster through Ansible or scripts.
With the power of Ansible these actions don't have to be limited to managing Kubernetes resources. We can expand to anything related to the cluster that Ansible can manage and coordinate in cluster action (things that Kubernetes have control over) with out of the cluster action (things that Kubernetes don't have control over, like infrastructure configuration).

## Let’s get started straight forward!

### Installing and using AnsibleAutomationPlatform

The following step is optional, as it certainly can be that you already use an Ansible-Tower or new Ansible Automation-Platform (AAP) instance which is not on the same Clusters as the ManagingCluster of RHACM.


In the following we will use some RHACM-Policies. Those policies will be applied using an Application which makes use of all artifacts we need.
They will be stored in a namespace called 'policies'.


Central-Component is the Policy-Generator-Config which you can review [here](https://github.com/ch-stark/maptoseveralstandards/blob/main/config/generic/generic-policygenerator.yaml).


So we start using:

- review the demo repository
- label the Hub-cluster

- apply [Managed-ClusterSetBinding-Policy][] 

```
oc create ns policies
oc apply -f https://raw.githubusercontent.com/stolostron/policy-collection/main/community/CM-Configuration-Management/policy-configure-subscription-admin-hub.yaml
policy.policy.open-cluster-management.io/policy-configure-subscription-admin-hub created
placementbinding.policy.open-cluster-management.io/binding-policy-configure-subscription-admin-hub created
placementrule.apps.open-cluster-management.io/placement-policy-configure-subscription-admin-hub created


Please note in this example we use the new Placement-API which requires a ClusterSetBinding to the namespace-policies.

oc apply -f 
managedclustersetbinding.cluster.open-cluster-management.io/default created

```

* After that proceed and ensure that the policy is applied (you might set it from `inform` to `enforce`)




For installing AAP on an OpenShift-Cluster you could als use the following [Policy](https://raw.githubusercontent.com/stolostron/policy-collection/main/community/CM-Configuration-Management/policy-automation-operator.yaml) directly from our Policy-Collection-Repository which is our central place for examples.

Basically the steps for AAP are:

* Create Operator, a Controller Instance and an Automation Hub. 
* complete the steps below to activate your subscription


After having the Operator installed and configured you will find the secrets in the namespace which you can use to login into the Controller and into the AutomationHub.


![alt text](images/image_secrets.png)


Here you see the routes which should have been created:

```
oc get routes -n ansible-automation-platform 
NAME             HOST/PORT                                                                                                   PATH   SERVICES                 PORT       TERMINATION     WILDCARD
automation-hub   automation-hub-ansible-automation-platform.apps.policy-grc-cp-autoclaims-4lmfj.dev08.red-chesterfield.com          automation-hub-web-svc   web-8080   edge/Redirect   None
controller       controller-ansible-automation-platform.apps.policy-grc-cp-autoclaims-4lmfj.dev08.red-chesterfield.com              controller-service       http       edge/Redirect   None
```


## Configure MultiClusterHub

In previous steps we used the following input [Policy-Input](https://github.com/ch-stark/maptoseveralstandards/blob/main/config/input/mulitclusterhub/MultiClusterHub.yaml) to configure the MultiClusterHub, the central component of RHACM:
 

Please note the following settings are especially necessary:

```
     - enabled: true
       name: cluster-proxy-addon
     - enabled: true
       name: managedserviceaccount-preview
```

In order to work the Policy must be configured with `MustOnlyHave` setting which has been applied in [PolicyGenerator file]((https://github.com/ch-stark/maptoseveralstandards/blob/main/config/generic/policyGenerator_generic.yaml#L53).).

Please learn more on the relevant features here.
 
* Managed-Service-Account

  https://open-cluster-management.io/getting-started/integration/managed-serviceaccount/

  This feature additionally needs to be enabled within the namespace of the Managed-Cluster on the hub. We also have a Policy for that. See here how it is configured: (https://github.com/ch-stark/maptoseveralstandards/blob/main/config/generic/policyGenerator_generic.yaml#L53).
 
* Cluster-Proxy

  https://open-cluster-management.io/getting-started/integration/cluster-proxy/
  
  If you check the Addons-Tab from your Managed-Cluster you should see the following content:



![alt text](images/addon.png)


### AAP Controller

As mentioned previously you can configure the Controller via Policy or Manually create it from the UI.  It will be installed into an ansible-automation-platform namespace.


![alt text](images/controller.png)


### AAP Private Hub
         
For more information please read:  https://access.redhat.com/documentation/en-us/red_hat_ansible_automation_platform/2.1/html/getting_started_with_automation_hub/index


![alt text](images/automationhub.png)


### Build and Upload ocmplus collection to AAP Hub 

Use the below command from the root folder of the repo : https://github.com/stolostron/ocmplus.cm.git to build the RHACM-Ansible collection for later usage.


```
ansible-galaxy collection build

Created collection for ocmplus.cm at ocmplus.cm/ocmplus-cm-0.0.1.tar.gz

```

* Then in Automation-Hub create a namespace and upload the files.


As you see in the following screenshot once you uploaded the file, it needs to be approved for getting it published into the private AutomationHub.


![alt text](images/approved.png)


At the end you should see the following content:


![alt text](images/collection_final.png)


#Note: Now you are finished with the setup of the Automation-Hub and let’s get back to the Controller again.


### Add Kubeconfig in Credential Types using config like below 


The following important steps are necessary to make a later connection to the Hub-Cluster.


![alt text](images/kubeconfig_creds.png)


```
---
fields:
  - id: kube_config
    type: string
    label: kubeconfig
    secret: true
    multiline: true
required:
  - kube_config


---
env:
  K8S_AUTH_KUBECONFIG: "{{ tower.filename.kubeconfig }}"
file:
  template.kubeconfig: "{{ kube_config }}"

```


In the next step add RHACM credentials by uploading the kubeconfig of the hub cluster.  During the tests it has been taken from the local `~.kube/config` file of the Hub cluster.


![alt text](images/kubeconfig.png)


Now you are finished setting up kubeconfig. You will need it several times in the following steps so ensure that it will be entered correctly.

### Add Credentials of the Ansible Automation Hub to the Ansible Controller 

The following step is necessary to enable Authentication from the Controller to the Automation-Hub.

Note: Ensure the above credentials end with /api/galaxy/content/published/ else you will not be able to make a successful call.


### Add the Private AAP-Hub to Galaxy-Credentials

Before we sync you also need to make sure below steps is done for adding the private automation hub as well in search by editing the organisation


![alt text](images/org_aaphub.png)



### Disable SSL on AAP Hub (Optional) 

If you don’t do this step below by default the Controller cannot call the Hub and you will not be able to sync the project.


![alt text](images/ssl_verification.png)



### Create a new project ACM Ansible Collection Demo and add below repo as Git Type 

We will make us of the following repository https://github.com/TheRealHaoLiu/acm-ansible-collection-demo.git
Feel free to fork it and modify it for your needs.

![alt text](images/project.png)


Ensure that the project is getting synced, this is a precondition for the next steps.


### Add Inventory, Inventory-Source and Sync

In the following we  firstconfigure  the inventory


![alt text](images/inventort_start.png)


At this step we add an Inventory-Source based on the previously defined project, an inventory-file (please review and adjust this to your environment) and the kubeconfig we generated before.

![alt text](images/inventory_source.png)


Now please synchronize and you will see Groups and Hosts being populated.
After you have successfully synced the inventory you should have seen the following logs:

```
ansible-inventory [core 2.12.1]
  config file = /etc/ansible/ansible.cfg
  configured module search path = ['/home/runner/.ansible/plugins/modules', '/usr/share/ansible/plugins/modules']
  ansible python module location = /usr/lib/python3.8/site-packages/ansible
  ansible collection location = 
   10.290 INFO     Processing JSON output...
   10.292 INFO     Loaded 7 groups, 1 hosts
   10.520 INFO     Inventory import completed for source in 0.3s
```

![alt text](images/group_synced.png)


#### Verification of previous work

Now please ensure once more all is working, you can review the following screenshots as a guideline.

We generated a Job-Template to test the previous setup:

![alt text](images/testjobsetup.png)


This template, using the Kubeconfig and which is build on the inventory and the project we generated will create a namespace
‘cool-app’ on all the Managed-Clusters. 


```
---
target_hosts: all-managed-clusters
state: present
namespace: cool-app
```


After launching the template you should see the following results:


![alt text](images/sync_end.png)


Verify the Cluster having the change active:

```
oc get ns -A |grep cool
cool-app                                           Active   43s
```



### Closing words


We highly recommend trying it out, many use cases come while seeing the features working.
You can embed any actions against a Cluster-Fleet now into your daily work and workflows and we are looking forward to any discussion regarding the use cases.
If you review this blog on [ACM to Ansible-Integration](https://cloud.redhat.com/blog/initiating-ansible-automation-on-policy-violations) 
you can now implement Automated Governance to highlight how to invoke AAP automation via ACM results which can first invoke complex workflows and then come back and make further modificiations on RHACM-fleet side.







Archive of old info 

Last year we had two session on Governance and AppLifecycle
This year we should talk about new Ansible-Collection


https://github.com/stolostron/blog-drafts/pull/102
https://github.com/stolostron/blog-drafts/blob/3bd639f0c3cb33aecad448272b0f28f3d928a229/ansible-collection/acm_ansible_blog.md


Speaker: Hao (onsite) Gurney

Note: last year we did two recordings

Title:
Get ready with Kubernetes MultiCluster Management using Ansible Automation Platform


If you are already working with Ansible Automation Platform (AAP) and you are either already using the [Ansible Kubernetes-Collection](https://github.com/ansible-collections/community.kubernetes) or you like to get started with OpenShift/Kubernetes then we now offer an Ansible-Collection which helps you to easily get started to Managed RedHat Advanced Cluster Management for Kubernetes (RHACM). In ACM 2.5 we release this as Dev-Preview and are working hard to make this Tech-Preview and GA in further releases.

You can control permissions and you automatically know about the state of each Cluster of your fleet as we maintain a dynamic inventory.

This will enable you to only focus entirely on the Business Logic.

In the following we are going to explain step by step how you can securely do any Kubernetes related Operation against any Cluster in the fleet which RHACM managed.

Kubernetes allows users to declaratively manage the desired state of the system. But much like any other declarative state management system, sometimes imperative and idempotent is needed to amplify its power and provide ordering and provide powerful templitization functionality.

The tools in the collection allows users of ACM to be able to connect with managed kubernetes clusters and do ANYTHING on the managed cluster through Ansible or scripts.
With the power of Ansible these actions don't have to be limited to managing Kubernetes resources. We can expand to anything related to the cluster that Ansible can manage and coordinate in cluster action (things that Kubernetes have control over) with out of the cluster action (things that Kubernetes don't have control over, like infrastructure configuration).
Ansible Galaxy Collection: https://galaxy.ansible.com/stolostron/core
Demo repo: https://github.com/stolostron/acm-ansible-collection-demo
