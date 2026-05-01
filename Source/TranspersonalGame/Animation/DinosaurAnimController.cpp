#include "DinosaurAnimController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/Engine.h"

UDinosaurAnimController::UDinosaurAnimController()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    CurrentState = EAnim_DinosaurState::Idle;
    Species = EAnim_DinosaurSpecies::TRex;
    MovementSpeed = 0.0f;
    MovementDirection = 0.0f;
    BlendWeight = 1.0f;
    SkeletalMeshComponent = nullptr;
    AnimInstance = nullptr;
}

void UDinosaurAnimController::BeginPlay()
{
    Super::BeginPlay();
    
    // Find the skeletal mesh component on the owner
    AActor* Owner = GetOwner();
    if (Owner)
    {
        SkeletalMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (SkeletalMeshComponent)
        {
            AnimInstance = SkeletalMeshComponent->GetAnimInstance();
            UE_LOG(LogTemp, Log, TEXT("DinosaurAnimController: Found SkeletalMeshComponent and AnimInstance"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("DinosaurAnimController: No SkeletalMeshComponent found on owner"));
        }
    }
    
    InitializeAnimationSets();
}

void UDinosaurAnimController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateAnimationParameters();
}

void UDinosaurAnimController::SetAnimationState(EAnim_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        EAnim_DinosaurState PreviousState = CurrentState;
        CurrentState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("DinosaurAnimController: State changed from %d to %d"), 
               (int32)PreviousState, (int32)CurrentState);
        
        // Handle state-specific logic
        switch (CurrentState)
        {
            case EAnim_DinosaurState::Attacking:
                PlayAttackAnimation();
                break;
            case EAnim_DinosaurState::Feeding:
                PlayFeedingAnimation();
                break;
            case EAnim_DinosaurState::Alert:
                PlayAlertAnimation();
                break;
            case EAnim_DinosaurState::Death:
                PlayDeathAnimation();
                break;
            default:
                break;
        }
    }
}

void UDinosaurAnimController::SetSpecies(EAnim_DinosaurSpecies NewSpecies)
{
    if (Species != NewSpecies)
    {
        Species = NewSpecies;
        UE_LOG(LogTemp, Log, TEXT("DinosaurAnimController: Species changed to %d"), (int32)Species);
        
        // Reinitialize animation sets for new species
        InitializeAnimationSets();
    }
}

void UDinosaurAnimController::PlayAttackAnimation()
{
    if (!AnimInstance || !SkeletalMeshComponent)
    {
        return;
    }
    
    FAnim_DinosaurAnimationSet* AnimSet = GetCurrentAnimationSet();
    if (AnimSet && AnimSet->AttackAnimation)
    {
        AnimInstance->Montage_Play(AnimSet->AttackAnimation);
        UE_LOG(LogTemp, Log, TEXT("DinosaurAnimController: Playing attack animation"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurAnimController: No attack animation found for species %d"), (int32)Species);
    }
}

void UDinosaurAnimController::PlayFeedingAnimation()
{
    if (!AnimInstance || !SkeletalMeshComponent)
    {
        return;
    }
    
    FAnim_DinosaurAnimationSet* AnimSet = GetCurrentAnimationSet();
    if (AnimSet && AnimSet->FeedingAnimation)
    {
        AnimInstance->Montage_Play(AnimSet->FeedingAnimation);
        UE_LOG(LogTemp, Log, TEXT("DinosaurAnimController: Playing feeding animation"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurAnimController: No feeding animation found for species %d"), (int32)Species);
    }
}

void UDinosaurAnimController::PlayAlertAnimation()
{
    if (!AnimInstance || !SkeletalMeshComponent)
    {
        return;
    }
    
    FAnim_DinosaurAnimationSet* AnimSet = GetCurrentAnimationSet();
    if (AnimSet && AnimSet->AlertAnimation)
    {
        AnimInstance->Montage_Play(AnimSet->AlertAnimation);
        UE_LOG(LogTemp, Log, TEXT("DinosaurAnimController: Playing alert animation"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurAnimController: No alert animation found for species %d"), (int32)Species);
    }
}

void UDinosaurAnimController::PlayDeathAnimation()
{
    if (!AnimInstance || !SkeletalMeshComponent)
    {
        return;
    }
    
    FAnim_DinosaurAnimationSet* AnimSet = GetCurrentAnimationSet();
    if (AnimSet && AnimSet->DeathAnimation)
    {
        AnimInstance->Montage_Play(AnimSet->DeathAnimation);
        UE_LOG(LogTemp, Log, TEXT("DinosaurAnimController: Playing death animation"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurAnimController: No death animation found for species %d"), (int32)Species);
    }
}

void UDinosaurAnimController::SetMovementSpeed(float Speed)
{
    MovementSpeed = FMath::Clamp(Speed, 0.0f, 1000.0f);
    
    // Update animation state based on speed
    if (MovementSpeed < 10.0f)
    {
        SetAnimationState(EAnim_DinosaurState::Idle);
    }
    else if (MovementSpeed < 300.0f)
    {
        SetAnimationState(EAnim_DinosaurState::Walking);
    }
    else
    {
        SetAnimationState(EAnim_DinosaurState::Running);
    }
}

void UDinosaurAnimController::SetMovementDirection(float Direction)
{
    MovementDirection = FMath::Clamp(Direction, -180.0f, 180.0f);
}

void UDinosaurAnimController::SetBlendWeight(float Weight)
{
    BlendWeight = FMath::Clamp(Weight, 0.0f, 1.0f);
}

void UDinosaurAnimController::InitializeAnimationSets()
{
    // Initialize default animation sets for each species
    // In a real implementation, these would be loaded from data assets
    
    FAnim_DinosaurAnimationSet TRexSet;
    // TRexSet animations would be assigned here
    AnimationSets.Add(EAnim_DinosaurSpecies::TRex, TRexSet);
    
    FAnim_DinosaurAnimationSet RaptorSet;
    // RaptorSet animations would be assigned here
    AnimationSets.Add(EAnim_DinosaurSpecies::Raptor, RaptorSet);
    
    FAnim_DinosaurAnimationSet TriceratopsSet;
    // TriceratopsSet animations would be assigned here
    AnimationSets.Add(EAnim_DinosaurSpecies::Triceratops, TriceratopsSet);
    
    FAnim_DinosaurAnimationSet BrachiosaurusSet;
    // BrachiosaurusSet animations would be assigned here
    AnimationSets.Add(EAnim_DinosaurSpecies::Brachiosaurus, BrachiosaurusSet);
    
    FAnim_DinosaurAnimationSet StegosaurusSet;
    // StegosaurusSet animations would be assigned here
    AnimationSets.Add(EAnim_DinosaurSpecies::Stegosaurus, StegosaurusSet);
    
    FAnim_DinosaurAnimationSet PteranodonSet;
    // PteranodonSet animations would be assigned here
    AnimationSets.Add(EAnim_DinosaurSpecies::Pteranodon, PteranodonSet);
    
    FAnim_DinosaurAnimationSet CompsognathusSet;
    // CompsognathusSet animations would be assigned here
    AnimationSets.Add(EAnim_DinosaurSpecies::Compsognathus, CompsognathusSet);
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurAnimController: Animation sets initialized for %d species"), AnimationSets.Num());
}

void UDinosaurAnimController::UpdateAnimationParameters()
{
    if (!AnimInstance)
    {
        return;
    }
    
    // Update animation parameters that can be accessed by Animation Blueprints
    // These would typically be set as variables in the AnimInstance
    
    // Example: Set movement speed for blend spaces
    // AnimInstance->SetVariableFloat("MovementSpeed", MovementSpeed);
    // AnimInstance->SetVariableFloat("MovementDirection", MovementDirection);
    // AnimInstance->SetVariableInt("DinosaurState", (int32)CurrentState);
}

FAnim_DinosaurAnimationSet* UDinosaurAnimController::GetCurrentAnimationSet()
{
    return AnimationSets.Find(Species);
}