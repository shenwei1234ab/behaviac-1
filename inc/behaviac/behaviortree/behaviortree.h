/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tencent is pleased to support the open source community by making behaviac available.
//
// Copyright (C) 2015 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except in compliance with
// the License. You may obtain a copy of the License at http://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef BEHAVIAC_BEHAVIORTREE_NODE_H
#define BEHAVIAC_BEHAVIORTREE_NODE_H

#include "behaviac/base/base.h"

#include "behaviac/base/dynamictype.h"
#include "behaviac/base/object/tagobject.h"
#include "behaviac/base/core/factory.h"

#include "behaviac/base/workspace.h"

#include "behaviac/property/property.h"

#include "behaviac/base/core/rapidxml/rapidxml.hpp"

/*! \mainpage Behaviac Help Home Page
\section secOverview Overview
Behaviac is a system, the purpose of which is to streamline the iterative process of designing,
integrating and debugging behavioral AI via behavior trees.Most behavior tree implementation,
though generally working under the same principles, differ slightly in their definition of the
practical method of behavior trees. if you are completely new to behavior trees then you are
supposed to google some papers to learn first.

The structure of a tree is defined by a number of interconnected control nodes, ending in leaf nodes actions.
Actions are the interfaces to your agent code. A btexe of a tree flows left to right, evaluating branching based
on tree state and then bottom to top, propagating result values and updating the tree state.

Each control node has its own rule set setting it apart defining which child node it ticks and how it responds to the
result of it Success/Failure or Running. In general, though, a result of Running (indicating more processing needed)
will always make sure that the child in question gets evaluated again next tick.

All nodes hold a tiny bit of state to indicate the current execution flow state such as the index of the last ticked
child for sequences and selectors. This state is updated in accordance with the rule-set of the node in question in
response to the result of the current tick.

With a few exceptions, once a node has updated its state, it also completes returning a result of Success/Failure or Running
to its parent node. Generally this means that any tree tick will result in at least one action node ticked.The result of
the root node is the result of the entire tree execution Running indicating that the tree has not completed and
Success/Failure indicating a complete run and its result.
*/

namespace behaviac
{
    const int INVALID_NODE_ID = -2;

    class Property;
    class Agent;
    class BehaviorTask;
    class BehaviorTreeTask;
    class Attachment;
    class BsonDeserizer;
    class Effector;
    class Precondition;
    class PlannerTaskComplex;
    class Planner;
    /**
    * Return values of exec/update and valid states for behaviors.
    */
    enum EBTStatus
    {
        BT_INVALID,
        BT_SUCCESS,
        BT_FAILURE,
        BT_RUNNING,
    };

    BEHAVIAC_API CMethodBase* LoadMethod(const char* value);
    class BsonDeserizer
    {
    public:
        enum BsonTypes
        {
            BT_None = 0,
            BT_Double = 1,
            BT_String = 2,
            BT_Object = 3,
            BT_Array = 4,
            BT_Binary = 5,
            BT_Undefined = 6,
            BT_ObjectId = 7,
            BT_Boolean = 8,
            BT_DateTime = 9,
            BT_NULL = 10,
            BT_Regex = 11,
            BT_Reference = 12,
            BT_Code = 13,
            BT_Symbol = 14,
            BT_ScopedCode = 15,
            BT_Int32 = 16,
            BT_Timestamp = 17,
            BT_Int64 = 18,
            BT_Float = 19,
            BT_Element = 20,
            BT_Set = 21,
            BT_BehaviorElement = 22,
            BT_PropertiesElement = 23,
            BT_ParsElement = 24,
            BT_ParElement = 25,
            BT_NodeElement = 26,
            BT_AttachmentsElement = 27,
            BT_AttachmentElement = 28,
            BT_AgentsElement = 29,
            BT_AgentElement = 30,
            BT_PropertyElement = 31,
            BT_MethodsElement = 32,
            BT_MethodElement = 33,
            BT_Custom = 34,
            BT_ParameterElement = 35
        };
    public:
        BsonDeserizer();
        virtual ~BsonDeserizer();

        bool Init(const char* pBuffer);

        bool OpenDocument();
        void CloseDocument(bool bEatEod = false);

        BsonTypes ReadType();
        int32_t ReadInt32();
        uint16_t ReadUInt16();
        float ReadFloat();
        bool ReadBool();
        const char* ReadString();

        bool eod() const;

    private:
        const char*		m_pBuffer;
        const char*		m_pPtr;
#if USE_DOCUMENET
        const char*		m_document;

        static const int kDocumentStackMax = 100;
        int				m_documentStackTop;
        const char*		m_documentStack[kDocumentStackMax];
#endif//USE_DOCUMENET
    };

    /**
    * Base class for BehaviorTree Nodes. This is the static part
    */
    class BEHAVIAC_API BehaviorNode : public CDynamicType
    {
    public:
        static const char* LOCAL_TASK_PARAM_PRE;
        enum EPhase
        {
            E_SUCCESS,
            E_FAILURE,
            E_BOTH
        };
        template<typename T>
        static bool Register()
        {
            Factory().Register<T>();

            return true;
        }

        template<typename T>
        static void UnRegister()
        {
            Factory().UnRegister<T>();
        }

        static BehaviorNode* Create(const char* className);
        virtual bool decompose(BehaviorNode* node, PlannerTaskComplex* seqTask, int depth, Planner* planner)
        {
            BEHAVIAC_UNUSED_VAR(node);
            BEHAVIAC_UNUSED_VAR(seqTask);
            BEHAVIAC_UNUSED_VAR(depth);
            BEHAVIAC_UNUSED_VAR(planner);
            BEHAVIAC_ASSERT(false, "Can't step into this line");
            return false;
        }
        static BehaviorNode* load(const char* agentType, rapidxml::xml_node<>* node, int version);

        static void Cleanup();

        static CFactory<BehaviorNode>& Factory();
        static void CombineResults(bool& firstValidPrecond, bool& lastCombineValue, Precondition* pPrecond, bool taskBoolean);

    public:
        BehaviorTask* CreateAndInitTask() const;
        bool HasEvents() const;
        void SetHasEvents(bool hasEvents);

        uint32_t GetChildrenCount() const;
        const BehaviorNode* GetChild(uint32_t index) const;
        BehaviorNode* GetChildById(int nodeId) const;
        uint32_t GetAttachmentsCount() const;
        const BehaviorNode* GetAttachment(uint32_t index) const;

        const BehaviorNode* GetParent() const
        {
            return this->m_parent;
        }

        void Clear();
        bool CheckPreconditions(const Agent* pAgent, bool bIsAlive) const;
        virtual void ApplyEffects(Agent* pAgent, BehaviorNode::EPhase phase) const;
        bool CheckEvents(const char* eventName, Agent* pAgent) const;
        virtual void Attach(BehaviorNode* pAttachment, bool bIsPrecondition, bool bIsEffector, bool bIsTransition);
        void Attach(BehaviorNode* pAttachment, bool bIsPrecondition, bool bIsEffector);

        virtual bool Evaluate(const Agent* pAgent)
        {
            BEHAVIAC_UNUSED_VAR(pAgent);
            BEHAVIAC_ASSERT(false, "Only Condition/Sequence/And/Or allowed");
            return false;
        }
        //return true for Parallel, SelectorLoop, etc., which is responsible to update all its children just like sub trees
        //so that they are treated as a return-running node and the next update will continue them.
        virtual bool IsManagingChildrenAsSubTrees() const;
        void InstantiatePars(Agent* pAgent) const;
        void UnInstantiatePars(Agent* pAgent) const;

    protected:
        BEHAVIAC_DECLARE_MEMORY_OPERATORS(BehaviorNode);
        BEHAVIAC_DECLARE_ROOT_DYNAMIC_TYPE(BehaviorNode, CDynamicType);

        BehaviorNode();
        virtual ~BehaviorNode();

        virtual bool IsValid(Agent* pAgent, BehaviorTask* pTask) const;

        virtual void load(int version, const char* agentType, const properties_t& properties);

        void load_par(int version, const char* agentType, rapidxml::xml_node<>* node);
        void load_properties(int version, const char* agentType, rapidxml::xml_node<>* node);
        void load_properties_pars(int version, const char* agentType, rapidxml::xml_node<>* node);
        bool load_property_pars(properties_t& properties, rapidxml::xml_node<>* c, int version, const char* agentType);
        bool load_attachment(int version, const char* agentType, bool bHasEvents, rapidxml::xml_node<>*  c);
        void load_properties_pars_attachments_children(bool bNode, int version, const char* agentType, rapidxml::xml_node<>* node);
        void load_attachment_transition_effectors(int version, const char* agentType, bool bHasEvents, rapidxml::xml_node<>* c);

        void load_par(int version, const char* agentType, BsonDeserizer& d);
        void load_pars(int version, const char* agentType, BsonDeserizer& d);
        void load_properties(int version, const char* agentType, BsonDeserizer& d);
        void load_attachments(int version, const char* agentType, BsonDeserizer& d, bool bIsTransition);
        void load_children(int version, const char* agentType, BsonDeserizer& d);
        void load_properties_pars_attachments_children(int version, const char* agentType, BsonDeserizer& d, bool bIsTransition);
        void load_custom(int version, const char* agentType, BsonDeserizer& d);
        BehaviorNode* load_node(int version, const char*  agentType, BsonDeserizer& d);

        BehaviorNode* load(const char* agentType, BsonDeserizer& d, int version);

        virtual void AddChild(BehaviorNode* pChild);
        virtual EBTStatus update_impl(Agent* pAgent, EBTStatus childStatus);

        void SetClassNameString(const char* className);
        const behaviac::string& GetClassNameString() const;

        int GetId() const;
        void SetId(int id);

        void SetAgentType(const behaviac::string& agentType);

        void AddPar(const char* agentType, const char* type, const char* name, const char* value);
        bool EvaluteCustomCondition(const Agent* pAgent);
        void SetCustomCondition(BehaviorNode* node);

    private:
        virtual BehaviorTask* createTask() const = 0;

        virtual bool enteraction_impl(Agent* pAgent)
        {
            BEHAVIAC_UNUSED_VAR(pAgent);
            return false;
        }
        virtual bool exitaction_impl(Agent* pAgent)
        {
            BEHAVIAC_UNUSED_VAR(pAgent);
            return false;
        }
    public:
        //return Preconditions' Count
        int PreconditionsCount() const;
    private:
        static CFactory<BehaviorNode>* ms_factory;
        behaviac::vector<BehaviorNode*>		m_preconditions;

        behaviac::string		m_className;
        int						m_id;
        behaviac::string		m_agentType;
        char					m_enter_precond;
        char					m_update_precond;
        char					m_both_precond;
        char					m_success_effectors;
        char					m_failure_effectors;
        char					m_both_effectors;
        behaviac::vector<BehaviorNode*>		m_effectors;
        behaviac::vector<BehaviorNode*>		m_events;
    protected:
        typedef behaviac::vector<BehaviorNode*> Attachments;
        Attachments*		m_attachments;

        typedef behaviac::vector<Property*> Properties_t;
        Properties_t*		m_pars;

        BehaviorNode*		m_parent;
        typedef behaviac::vector<BehaviorNode*> Nodes;
        Nodes*				m_children;
        BehaviorNode*		m_customCondition;

        CMethodBase*		m_enterAction;
        CMethodBase*		m_exitAction;

        bool				m_bHasEvents;
        bool				m_loadAttachment;
        friend class BehaviorTree;
        friend class BehaviorTask;
        friend class Agent;
    };

    class BEHAVIAC_API DecoratorNode : public BehaviorNode
    {
    public:
        BEHAVIAC_DECLARE_DYNAMIC_TYPE(DecoratorNode, BehaviorNode);

        DecoratorNode();
        virtual ~DecoratorNode();
        virtual void load(int version, const char* agentType, const properties_t& properties);
        virtual bool IsManagingChildrenAsSubTrees() const;
    protected:
        virtual bool IsValid(Agent* pAgent, BehaviorTask* pTask) const;

    private:
        //virtual BehaviorTask* createTask() const;

    protected:
        bool m_bDecorateWhenChildEnds;

        friend class DecoratorTask;
        friend class DecoratorRepeatTask;
    };

    // ============================================================================
    class BEHAVIAC_API BehaviorTree : public BehaviorNode
    {
    public:
        /**
        return the path relative to the workspace path
        */
        const behaviac::string& GetName() const
        {
            return this->m_name;
        }

        void SetName(const char* name)
        {
            this->m_name = name;
        }

        const behaviac::string& GetDomains() const;
        void SetDomains(const behaviac::string& domains);

        struct Descriptor_t
        {
            Property*			Descriptor;
            Property*			Reference;

            Descriptor_t() : Descriptor(0), Reference(0)
            {}

            Descriptor_t(const Descriptor_t& copy)
                : Descriptor(copy.Descriptor ? copy.Descriptor->clone() : NULL)
                , Reference(copy.Reference ? copy.Reference->clone() : NULL)
            {}

            ~Descriptor_t()
            {
                BEHAVIAC_DELETE(this->Descriptor);
                BEHAVIAC_DELETE(this->Reference);
            }

            DECLARE_BEHAVIAC_OBJECT_STRUCT(BehaviorTree::Descriptor_t);
        };

        typedef behaviac::vector<Descriptor_t>	Descriptors_t;
        const Descriptors_t GetDescriptors() const;
        void SetDescriptors(const char* descriptors);

        bool IsFSM();
        void SetIsFSM(bool isFsm);

    protected:
        BEHAVIAC_DECLARE_MEMORY_OPERATORS(BehaviorTree);
        BEHAVIAC_DECLARE_DYNAMIC_TYPE(BehaviorTree, BehaviorNode);
        BehaviorTree();
        virtual ~BehaviorTree();
        virtual void load(int version, const char* agentType, const properties_t& properties);

    private:
        virtual BehaviorTask* createTask() const;
        bool load_xml(char* pBuffer);
        bool load_bson(const char* pBuffer);

    protected:
        bool					m_bIsFSM;
        behaviac::string		m_name;
        behaviac::string		m_domains;
        Descriptors_t			m_descriptorRefs;

        friend class BehaviorTreeTask;
        friend class BehaviorNode;
        friend class Workspace;
    };
} // namespace behaviac

#endif//BEHAVIAC_BEHAVIORTREE_NODE_H