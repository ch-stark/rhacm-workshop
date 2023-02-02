VolSync-addon-controller is deployed to the “open-cluster-management” namespace in an ACM hub cluster.


VolSync can be deployed to managed clusters via a ManagedClusterAddOn resource.
VolSync is an operator/controller that can be installed on managed clusters to perform persistent volume replication within or across clusters.  The replication provided by VolSync is independent of the storage system. This allows replication to and from storage types that don’t normally support remote replication. Additionally, it can replicate across different types (and vendors) of storage. 
VolSync within ACM is managed as part of the Business Continuity squad. 
VolSync 0.6.0 will ship with ACM 2.7

Tesshu Flower
Documentation Link:
https://access.redhat.com/documentation/en-us/red_hat_advanced_cluster_management_for_kubernetes/2.7/html/add-ons/add-ons-overview#volsync

