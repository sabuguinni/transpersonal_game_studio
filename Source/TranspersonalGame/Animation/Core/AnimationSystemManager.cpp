#include "AnimationSystemManager.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "IKRigDefinition.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

UAnimationSystemManager::UAnimationSystemManager()
{
    LastUpdateTime = 0.0f;
    ActiveAnimationCount = 0;
}

void UAnimationSystemManager::InitializeAnimationSystem()
{
    UE_LOG(LogTemp, Log, TEXT("Animation System Manager: Initializing animation system"));
    
    // Initialize Motion Matching databases
    // These will be loaded from content assets
    
    // Initialize IK Rig assets
    // These will be loaded from content assets
    
    // Set up performance monitoring
    LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("Animation System Manager: Animation system initialized successfully"));
}

void UAnimationSystemManager::RegisterCharacter(AActor* Character, ECharacterType CharacterType)
{
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("Animation System Manager: Attempted to register null character"));
        return;
    }
    
    // Check if character is already registered
    if (RegisteredCharacters.Contains(Character))
    {
        UE_LOG(LogTemp, Warning, TEXT("Animation System Manager: Character already registered: %s"), *Character->GetName());
        return;
    }
    
    // Register the character
    RegisteredCharacters.Add(Character);
    
    // Set up animation blueprint based on character type
    USkeletalMeshComponent* SkeletalMesh = Character->FindComponentByClass<USkeletalMeshComponent>();
    if (SkeletalMesh && AnimBPClasses.Contains(CharacterType))
    {
        SkeletalMesh->SetAnimInstanceClass(AnimBPClasses[CharacterType]);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Animation System Manager: Registered character: %s as type: %d"), 
           *Character->GetName(), (int32)CharacterType);
}

void UAnimationSystemManager::UpdateAnimationSystem(float DeltaTime)
{
    SCOPE_CYCLE_COUNTER(STAT_AnimationSystemUpdate);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float ActualDeltaTime = CurrentTime - LastUpdateTime;
    LastUpdateTime = CurrentTime;
    
    ActiveAnimationCount = 0;
    
    // Update all registered characters
    for (AActor* Character : RegisteredCharacters)
    {
        if (!IsValid(Character))
        {
            continue;
        }
        
        // Update motion matching
        UpdateMotionMatching(Character, ActualDeltaTime);
        
        // Update IK system
        UpdateIKSystem(Character, ActualDeltaTime);
        
        // Apply procedural variations
        ApplyProceduralVariations(Character, ActualDeltaTime);
        
        ActiveAnimationCount++;
    }
    
    // Clean up invalid characters
    RegisteredCharacters.RemoveAll([](AActor* Character) {
        return !IsValid(Character);
    });
}

UPoseSearchDatabase* UAnimationSystemManager::GetMotionMatchingDatabase(ECharacterType CharacterType)
{
    if (MotionMatchingDatabases.Contains(CharacterType))
    {
        return MotionMatchingDatabases[CharacterType];
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Animation System Manager: No Motion Matching database found for character type: %d"), 
           (int32)CharacterType);
    return nullptr;
}

void UAnimationSystemManager::UpdateMotionMatching(AActor* Character, float DeltaTime)
{
    // Get the character's skeletal mesh component
    USkeletalMeshComponent* SkeletalMesh = Character->FindComponentByClass<USkeletalMeshComponent>();
    if (!SkeletalMesh)
    {
        return;
    }
    
    // Get the animation instance
    UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance();
    if (!AnimInstance)
    {
        return;
    }
    
    // Update motion matching parameters
    // This will be handled by the Animation Blueprint
    // We can set variables here that the AnimBP will read
    
    // Example: Set movement speed for motion matching
    if (ACharacter* CharacterPawn = Cast<ACharacter>(Character))
    {
        FVector Velocity = CharacterPawn->GetVelocity();
        float Speed = Velocity.Size();
        
        // Set speed variable in animation blueprint
        AnimInstance->SetVariableFloat(FName("MovementSpeed"), Speed);
        
        // Set direction variable
        FVector ForwardVector = Character->GetActorForwardVector();
        FVector RightVector = Character->GetActorRightVector();
        
        float ForwardSpeed = FVector::DotProduct(Velocity, ForwardVector);
        float RightSpeed = FVector::DotProduct(Velocity, RightVector);
        
        AnimInstance->SetVariableFloat(FName("ForwardSpeed"), ForwardSpeed);
        AnimInstance->SetVariableFloat(FName("RightSpeed"), RightSpeed);
    }
}

void UAnimationSystemManager::UpdateIKSystem(AActor* Character, float DeltaTime)
{
    // Update IK foot placement for terrain adaptation
    USkeletalMeshComponent* SkeletalMesh = Character->FindComponentByClass<USkeletalMeshComponent>();
    if (!SkeletalMesh)
    {
        return;
    }
    
    UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance();
    if (!AnimInstance)
    {
        return;
    }
    
    // Perform ground detection for foot IK
    FVector CharacterLocation = Character->GetActorLocation();
    FVector CharacterForward = Character->GetActorForwardVector();
    FVector CharacterRight = Character->GetActorRightVector();
    
    // Trace for left foot
    FVector LeftFootOffset = CharacterRight * -20.0f; // Adjust based on character
    FVector LeftFootStart = CharacterLocation + LeftFootOffset + FVector(0, 0, 50);
    FVector LeftFootEnd = LeftFootStart - FVector(0, 0, 150);
    
    FHitResult LeftFootHit;
    bool bLeftFootHit = GetWorld()->LineTraceSingleByChannel(
        LeftFootHit, LeftFootStart, LeftFootEnd, ECC_WorldStatic
    );
    
    // Trace for right foot
    FVector RightFootOffset = CharacterRight * 20.0f;
    FVector RightFootStart = CharacterLocation + RightFootOffset + FVector(0, 0, 50);
    FVector RightFootEnd = RightFootStart - FVector(0, 0, 150);
    
    FHitResult RightFootHit;
    bool bRightFootHit = GetWorld()->LineTraceSingleByChannel(
        RightFootHit, RightFootStart, RightFootEnd, ECC_WorldStatic
    );
    
    // Set IK variables in animation blueprint
    if (bLeftFootHit)
    {
        float LeftFootIKOffset = LeftFootHit.Location.Z - CharacterLocation.Z;
        AnimInstance->SetVariableFloat(FName("LeftFootIKOffset"), LeftFootIKOffset);
        AnimInstance->SetVariableBool(FName("LeftFootIKEnabled"), true);
    }
    else
    {
        AnimInstance->SetVariableBool(FName("LeftFootIKEnabled"), false);
    }
    
    if (bRightFootHit)
    {
        float RightFootIKOffset = RightFootHit.Location.Z - CharacterLocation.Z;
        AnimInstance->SetVariableFloat(FName("RightFootIKOffset"), RightFootIKOffset);
        AnimInstance->SetVariableBool(FName("RightFootIKEnabled"), true);
    }
    else
    {
        AnimInstance->SetVariableBool(FName("RightFootIKEnabled"), false);
    }
}

void UAnimationSystemManager::ApplyProceduralVariations(AActor* Character, float DeltaTime)
{
    // Apply subtle procedural variations to make each dinosaur unique
    USkeletalMeshComponent* SkeletalMesh = Character->FindComponentByClass<USkeletalMeshComponent>();
    if (!SkeletalMesh)
    {
        return;
    }
    
    UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance();
    if (!AnimInstance)
    {
        return;
    }
    
    // Generate unique variations based on character's unique ID
    int32 CharacterID = Character->GetUniqueID();
    FRandomStream RandomStream(CharacterID);
    
    // Apply gait variations (subtle differences in walking style)
    float GaitVariation = RandomStream.FRandRange(0.8f, 1.2f);
    AnimInstance->SetVariableFloat(FName("GaitVariation"), GaitVariation);
    
    // Apply posture variations (slight differences in stance)
    float PostureVariation = RandomStream.FRandRange(-5.0f, 5.0f);
    AnimInstance->SetVariableFloat(FName("PostureVariation"), PostureVariation);
    
    // Apply timing variations (slight differences in animation timing)
    float TimingVariation = RandomStream.FRandRange(0.9f, 1.1f);
    AnimInstance->SetVariableFloat(FName("TimingVariation"), TimingVariation);
}