#include "Anim_BlendTreeController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

UAnim_BlendTreeController::UAnim_BlendTreeController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    CurrentTreeIndex = 0;
    BlendTransitionSpeed = 5.0f;
    bAutoUpdateBlending = true;
    
    // Initialize movement parameters
    MovementSpeed = 0.0f;
    MovementDirection = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    
    TargetMesh = nullptr;
}

void UAnim_BlendTreeController::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-find skeletal mesh component if not set
    if (!TargetMesh)
    {
        TargetMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
        if (TargetMesh)
        {
            UE_LOG(LogTemp, Log, TEXT("BlendTreeController: Auto-found skeletal mesh component"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("BlendTreeController: No skeletal mesh component found"));
        }
    }
    
    // Initialize default blend tree if none exist
    if (BlendTrees.Num() == 0)
    {
        FAnim_BlendTree DefaultTree;
        DefaultTree.TreeName = TEXT("DefaultLocomotion");
        DefaultTree.bIsActive = true;
        BlendTrees.Add(DefaultTree);
        UE_LOG(LogTemp, Log, TEXT("BlendTreeController: Created default blend tree"));
    }
}

void UAnim_BlendTreeController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bAutoUpdateBlending && TargetMesh)
    {
        // Update movement parameters automatically
        UpdateMovementBlending();
        
        // Process blend trees
        ProcessBlendTrees(DeltaTime);
    }
}

void UAnim_BlendTreeController::AddBlendTree(const FAnim_BlendTree& NewTree)
{
    if (ValidateBlendTree(NewTree))
    {
        BlendTrees.Add(NewTree);
        UE_LOG(LogTemp, Log, TEXT("BlendTreeController: Added blend tree '%s'"), *NewTree.TreeName);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BlendTreeController: Failed to add invalid blend tree '%s'"), *NewTree.TreeName);
    }
}

void UAnim_BlendTreeController::RemoveBlendTree(int32 TreeIndex)
{
    if (BlendTrees.IsValidIndex(TreeIndex))
    {
        FString TreeName = BlendTrees[TreeIndex].TreeName;
        BlendTrees.RemoveAt(TreeIndex);
        
        // Adjust current tree index if necessary
        if (CurrentTreeIndex >= TreeIndex && CurrentTreeIndex > 0)
        {
            CurrentTreeIndex--;
        }
        
        UE_LOG(LogTemp, Log, TEXT("BlendTreeController: Removed blend tree '%s'"), *TreeName);
    }
}

void UAnim_BlendTreeController::SetActiveBlendTree(int32 TreeIndex)
{
    if (IsBlendTreeValid(TreeIndex))
    {
        // Deactivate current tree
        if (BlendTrees.IsValidIndex(CurrentTreeIndex))
        {
            BlendTrees[CurrentTreeIndex].bIsActive = false;
        }
        
        // Activate new tree
        CurrentTreeIndex = TreeIndex;
        BlendTrees[CurrentTreeIndex].bIsActive = true;
        
        UE_LOG(LogTemp, Log, TEXT("BlendTreeController: Set active blend tree to '%s'"), *BlendTrees[CurrentTreeIndex].TreeName);
    }
}

FAnim_BlendTree UAnim_BlendTreeController::GetCurrentBlendTree() const
{
    if (BlendTrees.IsValidIndex(CurrentTreeIndex))
    {
        return BlendTrees[CurrentTreeIndex];
    }
    
    return FAnim_BlendTree();
}

void UAnim_BlendTreeController::AddBlendNode(int32 TreeIndex, const FAnim_BlendNode& NewNode)
{
    if (BlendTrees.IsValidIndex(TreeIndex) && ValidateBlendNode(NewNode))
    {
        BlendTrees[TreeIndex].BlendNodes.Add(NewNode);
        UE_LOG(LogTemp, Log, TEXT("BlendTreeController: Added blend node '%s' to tree '%s'"), 
               *NewNode.NodeName, *BlendTrees[TreeIndex].TreeName);
    }
}

void UAnim_BlendTreeController::UpdateBlendWeight(int32 TreeIndex, int32 NodeIndex, float NewWeight)
{
    if (BlendTrees.IsValidIndex(TreeIndex) && BlendTrees[TreeIndex].BlendNodes.IsValidIndex(NodeIndex))
    {
        BlendTrees[TreeIndex].BlendNodes[NodeIndex].BlendWeight = FMath::Clamp(NewWeight, 0.0f, 1.0f);
    }
}

void UAnim_BlendTreeController::UpdatePlayRate(int32 TreeIndex, int32 NodeIndex, float NewRate)
{
    if (BlendTrees.IsValidIndex(TreeIndex) && BlendTrees[TreeIndex].BlendNodes.IsValidIndex(NodeIndex))
    {
        BlendTrees[TreeIndex].BlendNodes[NodeIndex].PlayRate = FMath::Max(NewRate, 0.1f);
    }
}

void UAnim_BlendTreeController::UpdateMovementParameters(float Speed, float Direction, bool InAir, bool Crouching)
{
    MovementSpeed = Speed;
    MovementDirection = Direction;
    bIsInAir = InAir;
    bIsCrouching = Crouching;
}

void UAnim_BlendTreeController::SetBlendSpaceParameters(UBlendSpace* BlendSpace, float X, float Y)
{
    if (BlendSpace && TargetMesh && TargetMesh->GetAnimInstance())
    {
        // This would typically be handled in the Animation Blueprint
        // Here we log the parameters for debugging
        UE_LOG(LogTemp, Log, TEXT("BlendTreeController: Setting BlendSpace parameters X=%.2f, Y=%.2f"), X, Y);
    }
}

void UAnim_BlendTreeController::SetBlendSpace1DParameter(UBlendSpace1D* BlendSpace1D, float Value)
{
    if (BlendSpace1D && TargetMesh && TargetMesh->GetAnimInstance())
    {
        UE_LOG(LogTemp, Log, TEXT("BlendTreeController: Setting BlendSpace1D parameter Value=%.2f"), Value);
    }
}

void UAnim_BlendTreeController::PlayAnimationMontage(UAnimMontage* Montage, float PlayRate)
{
    if (Montage && TargetMesh && TargetMesh->GetAnimInstance())
    {
        TargetMesh->GetAnimInstance()->Montage_Play(Montage, PlayRate);
        UE_LOG(LogTemp, Log, TEXT("BlendTreeController: Playing animation montage '%s'"), *Montage->GetName());
    }
}

void UAnim_BlendTreeController::StopAnimationMontage(UAnimMontage* Montage)
{
    if (Montage && TargetMesh && TargetMesh->GetAnimInstance())
    {
        TargetMesh->GetAnimInstance()->Montage_Stop(0.2f, Montage);
        UE_LOG(LogTemp, Log, TEXT("BlendTreeController: Stopped animation montage '%s'"), *Montage->GetName());
    }
}

void UAnim_BlendTreeController::InitializeWithSkeletalMesh(USkeletalMeshComponent* SkeletalMesh)
{
    TargetMesh = SkeletalMesh;
    if (TargetMesh)
    {
        UE_LOG(LogTemp, Log, TEXT("BlendTreeController: Initialized with skeletal mesh '%s'"), *TargetMesh->GetName());
    }
}

bool UAnim_BlendTreeController::IsBlendTreeValid(int32 TreeIndex) const
{
    return BlendTrees.IsValidIndex(TreeIndex);
}

void UAnim_BlendTreeController::ResetAllBlendWeights()
{
    for (FAnim_BlendTree& Tree : BlendTrees)
    {
        for (FAnim_BlendNode& Node : Tree.BlendNodes)
        {
            Node.BlendWeight = 0.0f;
        }
    }
    UE_LOG(LogTemp, Log, TEXT("BlendTreeController: Reset all blend weights"));
}

void UAnim_BlendTreeController::PauseAllAnimations()
{
    if (TargetMesh && TargetMesh->GetAnimInstance())
    {
        // Pause all montages
        TargetMesh->GetAnimInstance()->Montage_Pause();
        UE_LOG(LogTemp, Log, TEXT("BlendTreeController: Paused all animations"));
    }
}

void UAnim_BlendTreeController::ResumeAllAnimations()
{
    if (TargetMesh && TargetMesh->GetAnimInstance())
    {
        // Resume all montages
        TargetMesh->GetAnimInstance()->Montage_Resume(nullptr);
        UE_LOG(LogTemp, Log, TEXT("BlendTreeController: Resumed all animations"));
    }
}

void UAnim_BlendTreeController::ProcessBlendTrees(float DeltaTime)
{
    if (!BlendTrees.IsValidIndex(CurrentTreeIndex))
    {
        return;
    }
    
    FAnim_BlendTree& CurrentTree = BlendTrees[CurrentTreeIndex];
    if (!CurrentTree.bIsActive)
    {
        return;
    }
    
    // Update blend weights based on movement
    UpdateBlendWeights(DeltaTime);
    
    // Apply blending to mesh
    ApplyBlendToMesh();
}

void UAnim_BlendTreeController::UpdateBlendWeights(float DeltaTime)
{
    if (!BlendTrees.IsValidIndex(CurrentTreeIndex))
    {
        return;
    }
    
    FAnim_BlendTree& CurrentTree = BlendTrees[CurrentTreeIndex];
    
    // Simple locomotion blending based on speed
    for (FAnim_BlendNode& Node : CurrentTree.BlendNodes)
    {
        if (Node.NodeName.Contains(TEXT("Idle")))
        {
            float TargetWeight = (MovementSpeed < 10.0f) ? 1.0f : 0.0f;
            Node.BlendWeight = FMath::FInterpTo(Node.BlendWeight, TargetWeight, DeltaTime, BlendTransitionSpeed);
        }
        else if (Node.NodeName.Contains(TEXT("Walk")))
        {
            float TargetWeight = (MovementSpeed >= 10.0f && MovementSpeed < 300.0f) ? 1.0f : 0.0f;
            Node.BlendWeight = FMath::FInterpTo(Node.BlendWeight, TargetWeight, DeltaTime, BlendTransitionSpeed);
        }
        else if (Node.NodeName.Contains(TEXT("Run")))
        {
            float TargetWeight = (MovementSpeed >= 300.0f) ? 1.0f : 0.0f;
            Node.BlendWeight = FMath::FInterpTo(Node.BlendWeight, TargetWeight, DeltaTime, BlendTransitionSpeed);
        }
        else if (Node.NodeName.Contains(TEXT("Jump")) && bIsInAir)
        {
            Node.BlendWeight = FMath::FInterpTo(Node.BlendWeight, 1.0f, DeltaTime, BlendTransitionSpeed * 2.0f);
        }
    }
}

void UAnim_BlendTreeController::ApplyBlendToMesh()
{
    // This is where we would apply the calculated blend weights to the actual animation system
    // In a full implementation, this would interface with the Animation Blueprint or AnimGraph
    if (TargetMesh && TargetMesh->GetAnimInstance())
    {
        // Log current blend state for debugging
        if (BlendTrees.IsValidIndex(CurrentTreeIndex))
        {
            const FAnim_BlendTree& CurrentTree = BlendTrees[CurrentTreeIndex];
            for (const FAnim_BlendNode& Node : CurrentTree.BlendNodes)
            {
                if (Node.BlendWeight > 0.01f)
                {
                    UE_LOG(LogTemp, VeryVerbose, TEXT("BlendTreeController: Node '%s' weight: %.3f"), 
                           *Node.NodeName, Node.BlendWeight);
                }
            }
        }
    }
}

void UAnim_BlendTreeController::UpdateMovementBlending()
{
    // Auto-update movement parameters from character
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            FVector Velocity = MovementComp->Velocity;
            MovementSpeed = Velocity.Size();
            
            // Calculate movement direction relative to actor forward
            if (MovementSpeed > 0.1f)
            {
                FVector ForwardVector = GetOwner()->GetActorForwardVector();
                FVector NormalizedVelocity = Velocity.GetSafeNormal();
                MovementDirection = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, NormalizedVelocity)));
            }
            else
            {
                MovementDirection = 0.0f;
            }
            
            bIsInAir = MovementComp->IsFalling();
            bIsCrouching = MovementComp->IsCrouching();
        }
    }
}

void UAnim_BlendTreeController::CalculateLocomotionBlend()
{
    // Advanced locomotion blending calculations
    // This could include directional movement, slope adaptation, etc.
    
    if (!BlendTrees.IsValidIndex(CurrentTreeIndex))
    {
        return;
    }
    
    FAnim_BlendTree& CurrentTree = BlendTrees[CurrentTreeIndex];
    
    // Normalize all blend weights to ensure they sum to 1.0
    float TotalWeight = 0.0f;
    for (const FAnim_BlendNode& Node : CurrentTree.BlendNodes)
    {
        TotalWeight += Node.BlendWeight;
    }
    
    if (TotalWeight > 0.0f)
    {
        for (FAnim_BlendNode& Node : CurrentTree.BlendNodes)
        {
            Node.BlendWeight /= TotalWeight;
        }
    }
}

bool UAnim_BlendTreeController::ValidateBlendTree(const FAnim_BlendTree& Tree) const
{
    if (Tree.TreeName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("BlendTreeController: Blend tree has empty name"));
        return false;
    }
    
    // Check for duplicate tree names
    for (const FAnim_BlendTree& ExistingTree : BlendTrees)
    {
        if (ExistingTree.TreeName == Tree.TreeName)
        {
            UE_LOG(LogTemp, Warning, TEXT("BlendTreeController: Blend tree name '%s' already exists"), *Tree.TreeName);
            return false;
        }
    }
    
    return true;
}

bool UAnim_BlendTreeController::ValidateBlendNode(const FAnim_BlendNode& Node) const
{
    if (Node.NodeName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("BlendTreeController: Blend node has empty name"));
        return false;
    }
    
    if (Node.PlayRate <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("BlendTreeController: Blend node '%s' has invalid play rate"), *Node.NodeName);
        return false;
    }
    
    return true;
}