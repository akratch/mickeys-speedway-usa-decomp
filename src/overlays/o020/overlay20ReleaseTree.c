#include "PR/ultratypes.h"

typedef struct Overlay20TreeNode {
    u8 pad00[0x20];
    struct Overlay20TreeNode *child0;
    struct Overlay20TreeNode *child1;
    struct Overlay20TreeNode *child2;
} Overlay20TreeNode;

void overlay20ReleaseTree(Overlay20TreeNode *node);
void overlay20ReleaseTreeReloc(Overlay20TreeNode *node);

/* DKR v77/v80 and JFG contain no exact donor for this recursive tree release. */
void overlay20ReleaseTree(Overlay20TreeNode *node) {
    if (node != NULL) {
        if (node->child0 != NULL) {
            overlay20ReleaseTree(node->child0);
            node->child0 = NULL;
        }
        if (node->child1 != NULL) {
            overlay20ReleaseTree(node->child1);
            node->child1 = NULL;
        }
        if (node->child2 != NULL) {
            overlay20ReleaseTree(node->child2);
            node->child2 = NULL;
        }
        overlay20ReleaseTreeReloc(node);
    }
}
