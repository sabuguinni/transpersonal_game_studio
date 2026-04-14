#include "AnimationSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/CollisionProfile.h"

UAnimationSystemManager::UAnimationSystemManager()
{
    IKTraceDistance = 50.0f;
    IKInterpSpeed = 15.0f;
    bEnableMotionMatching = true;
    bEnableFootIK = true;
    bDrawDebugInfo = false;
}

void UAnimationSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Animation System Manager initialized"));
    
    // Initialize motion database
    InitializeMotionDatabase();
    
    // Load animation assets
    LoadAnimationAssets();
}

void UAnimationSystemManager::Deinitialize()
{
    // Clear registered characters
    RegisteredCharacters.Empty();
    MotionDatabase.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Animation System Manager deinitialized"));
    
    Super::Deinitialize();
}

bool UAnimationSystemManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UAnimationSystemManager::RegisterCharacter(AActor* Character, EAnim_CharacterType CharacterType)
{
    if (!Character)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot register null character"));
        return;
    }

    FAnim_CharacterProfile Profile;
    Profile.CharacterType = CharacterType;
    Profile.CharacterName = Character->GetName();
    
    // Set character-specific parameters based on type
    switch (CharacterType)
    {
        case EAnim_CharacterType::Player:
            Profile.WalkSpeed = 150.0f;
            Profile.RunSpeed = 400.0f;
            Profile.PersonalityFactor = 0.7f; // Confident
            break;
        case EAnim_CharacterType::Hunter:
            Profile.WalkSpeed = 140.0f;
            Profile.RunSpeed = 380.0f;
            Profile.PersonalityFactor = 0.8f; // Aggressive
            break;
        case EAnim_CharacterType::Gatherer:
            Profile.WalkSpeed = 120.0f;
            Profile.RunSpeed = 320.0f;
            Profile.PersonalityFactor = 0.4f; // Calm
            break;
        case EAnim_CharacterType::Shaman:
            Profile.WalkSpeed = 100.0f;
            Profile.RunSpeed = 250.0f;
            Profile.PersonalityFactor = 0.3f; // Serene
            break;
        case EAnim_CharacterType::Child:
            Profile.WalkSpeed = 180.0f;
            Profile.RunSpeed = 350.0f;
            Profile.PersonalityFactor = 0.9f; // Energetic
            break;
        case EAnim_CharacterType::Elder:
            Profile.WalkSpeed = 80.0f;
            Profile.RunSpeed = 200.0f;
            Profile.PersonalityFactor = 0.2f; // Slow and deliberate
            break;
        default:
            Profile.WalkSpeed = 150.0f;
            Profile.RunSpeed = 400.0f;
            Profile.PersonalityFactor = 0.5f;
            break;
    }
    
    RegisteredCharacters.Add(Character, Profile);
    
    UE_LOG(LogTemp, Log, TEXT("Registered character: %s as %s"), 
           *Character->GetName(), 
           *UEnum::GetValueAsString(CharacterType));
}

void UAnimationSystemManager::UnregisterCharacter(AActor* Character)
{
    if (RegisteredCharacters.Contains(Character))
    {
        RegisteredCharacters.Remove(Character);
        UE_LOG(LogTemp, Log, TEXT("Unregistered character: %s"), *Character->GetName());
    }
}

FAnim_MotionData UAnimationSystemManager::CalculateMotionData(AActor* Character)
{
    FAnim_MotionData MotionData;
    
    if (!Character)
    {
        return MotionData;
    }
    
    // Get character velocity
    MotionData.Velocity = GetCharacterVelocity(Character);
    MotionData.Speed = MotionData.Velocity.Size();
    
    // Calculate movement direction relative to character forward
    MotionData.Direction = CalculateMovementDirection(Character);
    
    // Determine if character is accelerating
    static TMap<AActor*, FVector> PreviousVelocities;
    if (PreviousVelocities.Contains(Character))
    {
        FVector PrevVel = PreviousVelocities[Character];
        MotionData.bIsAccelerating = (MotionData.Velocity - PrevVel).SizeSquared() > 100.0f;
    }
    PreviousVelocities.Add(Character, MotionData.Velocity);
    
    // Check if character is in air
    MotionData.bIsInAir = IsCharacterInAir(Character);
    
    // Determine movement state
    MotionData.MovementState = DetermineMovementState(MotionData);
    
    return MotionData;
}

FAnim_IKData UAnimationSystemManager::CalculateFootIK(AActor* Character, USkeletalMeshComponent* SkeletalMesh)
{
    FAnim_IKData IKData;
    
    if (!Character || !SkeletalMesh || !bEnableFootIK)
    {
        return IKData;
    }
    
    // Get foot bone locations
    FVector LeftFootBoneLocation = SkeletalMesh->GetBoneLocation(TEXT("foot_l"));
    FVector RightFootBoneLocation = SkeletalMesh->GetBoneLocation(TEXT("foot_r"));
    
    // Trace for ground placement
    IKData.LeftFootLocation = TraceFootPlacement(Character, LeftFootBoneLocation);
    IKData.RightFootLocation = TraceFootPlacement(Character, RightFootBoneLocation);
    
    // Calculate foot rotations based on ground normal
    // This is a simplified version - in production you'd use the hit normal
    IKData.LeftFootRotation = FRotator::ZeroRotator;
    IKData.RightFootRotation = FRotator::ZeroRotator;
    
    // Calculate hip offset to keep character grounded
    float LeftOffset = IKData.LeftFootLocation.Z - LeftFootBoneLocation.Z;
    float RightOffset = IKData.RightFootLocation.Z - RightFootBoneLocation.Z;
    IKData.HipOffset = FMath::Min(LeftOffset, RightOffset);
    
    return IKData;
}

void UAnimationSystemManager::UpdateCharacterAnimation(AActor* Character, float DeltaTime)
{
    if (!Character || !RegisteredCharacters.Contains(Character))
    {
        return;
    }
    
    // Calculate motion data
    FAnim_MotionData MotionData = CalculateMotionData(Character);
    
    // Get skeletal mesh component
    USkeletalMeshComponent* SkeletalMesh = GetCharacterSkeletalMesh(Character);
    if (!SkeletalMesh)
    {
        return;
    }
    
    // Calculate IK data
    FAnim_IKData IKData = CalculateFootIK(Character, SkeletalMesh);
    
    // Find best matching animation if motion matching is enabled
    if (bEnableMotionMatching)
    {
        FAnim_CharacterProfile& Profile = RegisteredCharacters[Character];
        UAnimSequence* BestMatch = FindBestMatchingAnimation(MotionData, Profile.CharacterType);
        
        if (BestMatch)
        {
            // Apply the animation (this would normally be done through Animation Blueprint)
            UE_LOG(LogTemp, Verbose, TEXT("Best animation match for %s: %s"), 
                   *Character->GetName(), *BestMatch->GetName());
        }
    }
    
    // Draw debug info if enabled
    if (bDrawDebugInfo)
    {
        DrawMotionDebugInfo(Character);
        DrawIKDebugInfo(Character);
    }
}

UAnimSequence* UAnimationSystemManager::FindBestMatchingAnimation(const FAnim_MotionData& MotionData, EAnim_CharacterType CharacterType)
{
    if (!MotionDatabase.Contains(CharacterType))
    {
        return nullptr;
    }
    
    const TArray<UAnimSequence*>& Animations = MotionDatabase[CharacterType];
    if (Animations.Num() == 0)
    {
        return nullptr;
    }
    
    // Simple motion matching - in production this would be much more sophisticated
    // For now, just return based on movement state
    switch (MotionData.MovementState)
    {
        case EAnim_MovementState::Idle:
            return Animations.Num() > 0 ? Animations[0] : nullptr;
        case EAnim_MovementState::Walking:
            return Animations.Num() > 1 ? Animations[1] : nullptr;
        case EAnim_MovementState::Running:
            return Animations.Num() > 2 ? Animations[2] : nullptr;
        default:
            return Animations.Num() > 0 ? Animations[0] : nullptr;
    }
}

void UAnimationSystemManager::BuildMotionDatabase()
{
    UE_LOG(LogTemp, Warning, TEXT("Building motion database..."));
    
    // Clear existing database
    MotionDatabase.Empty();
    
    // This would normally load animations from content browser
    // For now, we'll create empty arrays for each character type
    for (int32 i = 0; i < (int32)EAnim_CharacterType::Dinosaur_Large + 1; i++)
    {
        EAnim_CharacterType CharType = (EAnim_CharacterType)i;
        MotionDatabase.Add(CharType, TArray<UAnimSequence*>());
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Motion database built with %d character types"), MotionDatabase.Num());
}

void UAnimationSystemManager::SetCharacterPersonality(AActor* Character, float PersonalityFactor)
{
    if (RegisteredCharacters.Contains(Character))
    {
        RegisteredCharacters[Character].PersonalityFactor = FMath::Clamp(PersonalityFactor, 0.0f, 1.0f);
    }
}

FAnim_CharacterProfile UAnimationSystemManager::GetCharacterProfile(AActor* Character)
{
    if (RegisteredCharacters.Contains(Character))
    {
        return RegisteredCharacters[Character];
    }
    
    return FAnim_CharacterProfile();
}

FVector UAnimationSystemManager::TraceFootPlacement(AActor* Character, FVector FootWorldLocation)
{
    if (!Character)
    {
        return FootWorldLocation;
    }
    
    UWorld* World = Character->GetWorld();
    if (!World)
    {
        return FootWorldLocation;
    }
    
    // Trace downward from foot location
    FVector Start = FootWorldLocation + FVector(0, 0, 20.0f);
    FVector End = FootWorldLocation - FVector(0, 0, IKTraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Character);
    
    if (World->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, QueryParams))
    {
        return HitResult.Location;
    }
    
    return FootWorldLocation;
}

void UAnimationSystemManager::EnableFootIK(AActor* Character, bool bEnable)
{
    if (RegisteredCharacters.Contains(Character))
    {
        // This would normally communicate with the Animation Blueprint
        UE_LOG(LogTemp, Log, TEXT("Foot IK %s for character: %s"), 
               bEnable ? TEXT("enabled") : TEXT("disabled"), 
               *Character->GetName());
    }
}

void UAnimationSystemManager::PlayCharacterMontage(AActor* Character, UAnimMontage* Montage, float PlayRate)
{
    if (!Character || !Montage)
    {
        return;
    }
    
    USkeletalMeshComponent* SkeletalMesh = GetCharacterSkeletalMesh(Character);
    if (!SkeletalMesh)
    {
        return;
    }
    
    UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance();
    if (AnimInstance)
    {
        AnimInstance->Montage_Play(Montage, PlayRate);
        UE_LOG(LogTemp, Log, TEXT("Playing montage %s on character %s"), 
               *Montage->GetName(), *Character->GetName());
    }
}

void UAnimationSystemManager::StopCharacterMontage(AActor* Character, float BlendOutTime)
{
    if (!Character)
    {
        return;
    }
    
    USkeletalMeshComponent* SkeletalMesh = GetCharacterSkeletalMesh(Character);
    if (!SkeletalMesh)
    {
        return;
    }
    
    UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance();
    if (AnimInstance)
    {
        AnimInstance->Montage_Stop(BlendOutTime);
        UE_LOG(LogTemp, Log, TEXT("Stopped montage on character %s"), *Character->GetName());
    }
}

void UAnimationSystemManager::DebugAnimationSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Animation System Debug ==="));
    UE_LOG(LogTemp, Warning, TEXT("Registered Characters: %d"), RegisteredCharacters.Num());
    UE_LOG(LogTemp, Warning, TEXT("Motion Database Entries: %d"), MotionDatabase.Num());
    UE_LOG(LogTemp, Warning, TEXT("Motion Matching Enabled: %s"), bEnableMotionMatching ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Foot IK Enabled: %s"), bEnableFootIK ? TEXT("Yes") : TEXT("No"));
    
    for (const auto& CharacterPair : RegisteredCharacters)
    {
        AActor* Character = CharacterPair.Key;
        const FAnim_CharacterProfile& Profile = CharacterPair.Value;
        
        UE_LOG(LogTemp, Warning, TEXT("Character: %s, Type: %s, Personality: %.2f"), 
               *Character->GetName(),
               *UEnum::GetValueAsString(Profile.CharacterType),
               Profile.PersonalityFactor);
    }
}

void UAnimationSystemManager::DrawMotionDebugInfo(AActor* Character)
{
    if (!Character)
    {
        return;
    }
    
    UWorld* World = Character->GetWorld();
    if (!World)
    {
        return;
    }
    
    FAnim_MotionData MotionData = CalculateMotionData(Character);
    FVector CharLocation = Character->GetActorLocation();
    
    // Draw velocity vector
    DrawDebugDirectionalArrow(World, CharLocation, CharLocation + MotionData.Velocity, 
                             50.0f, FColor::Red, false, 0.0f, 0, 2.0f);
    
    // Draw speed text
    FString SpeedText = FString::Printf(TEXT("Speed: %.1f"), MotionData.Speed);
    DrawDebugString(World, CharLocation + FVector(0, 0, 100), SpeedText, nullptr, FColor::White, 0.0f);
}

void UAnimationSystemManager::DrawIKDebugInfo(AActor* Character)
{
    if (!Character)
    {
        return;
    }
    
    UWorld* World = Character->GetWorld();
    if (!World)
    {
        return;
    }
    
    USkeletalMeshComponent* SkeletalMesh = GetCharacterSkeletalMesh(Character);
    if (!SkeletalMesh)
    {
        return;
    }
    
    FAnim_IKData IKData = CalculateFootIK(Character, SkeletalMesh);
    
    // Draw foot IK targets
    DrawDebugSphere(World, IKData.LeftFootLocation, 5.0f, 8, FColor::Green, false, 0.0f);
    DrawDebugSphere(World, IKData.RightFootLocation, 5.0f, 8, FColor::Blue, false, 0.0f);
    
    // Draw hip offset
    FVector CharLocation = Character->GetActorLocation();
    FString HipText = FString::Printf(TEXT("Hip Offset: %.1f"), IKData.HipOffset);
    DrawDebugString(World, CharLocation + FVector(0, 0, 120), HipText, nullptr, FColor::Yellow, 0.0f);
}

void UAnimationSystemManager::InitializeMotionDatabase()
{
    BuildMotionDatabase();
}

void UAnimationSystemManager::LoadAnimationAssets()
{
    // This would normally load animation assets from the content browser
    // For now, we'll just log that we're loading
    UE_LOG(LogTemp, Warning, TEXT("Loading animation assets..."));
}

USkeletalMeshComponent* UAnimationSystemManager::GetCharacterSkeletalMesh(AActor* Character)
{
    if (!Character)
    {
        return nullptr;
    }
    
    // Try to get skeletal mesh component
    USkeletalMeshComponent* SkeletalMesh = Character->FindComponentByClass<USkeletalMeshComponent>();
    
    // If it's a character, try to get the mesh component
    if (!SkeletalMesh)
    {
        ACharacter* CharacterPawn = Cast<ACharacter>(Character);
        if (CharacterPawn)
        {
            SkeletalMesh = CharacterPawn->GetMesh();
        }
    }
    
    return SkeletalMesh;
}

FVector UAnimationSystemManager::GetCharacterVelocity(AActor* Character)
{
    if (!Character)
    {
        return FVector::ZeroVector;
    }
    
    // Try to get velocity from character movement component
    ACharacter* CharacterPawn = Cast<ACharacter>(Character);
    if (CharacterPawn && CharacterPawn->GetCharacterMovement())
    {
        return CharacterPawn->GetCharacterMovement()->Velocity;
    }
    
    // Fallback to actor velocity
    return Character->GetVelocity();
}

bool UAnimationSystemManager::IsCharacterInAir(AActor* Character)
{
    if (!Character)
    {
        return false;
    }
    
    ACharacter* CharacterPawn = Cast<ACharacter>(Character);
    if (CharacterPawn && CharacterPawn->GetCharacterMovement())
    {
        return CharacterPawn->GetCharacterMovement()->IsFalling();
    }
    
    return false;
}

float UAnimationSystemManager::CalculateMovementDirection(AActor* Character)
{
    if (!Character)
    {
        return 0.0f;
    }
    
    FVector Velocity = GetCharacterVelocity(Character);
    if (Velocity.SizeSquared() < 1.0f)
    {
        return 0.0f;
    }
    
    FVector Forward = Character->GetActorForwardVector();
    FVector VelNormalized = Velocity.GetSafeNormal();
    
    float DotProduct = FVector::DotProduct(Forward, VelNormalized);
    float CrossProduct = FVector::CrossProduct(Forward, VelNormalized).Z;
    
    return FMath::Atan2(CrossProduct, DotProduct) * 180.0f / PI;
}

EAnim_MovementState UAnimationSystemManager::DetermineMovementState(const FAnim_MotionData& MotionData)
{
    if (MotionData.bIsInAir)
    {
        return MotionData.Velocity.Z > 0 ? EAnim_MovementState::Jumping : EAnim_MovementState::Falling;
    }
    
    if (MotionData.Speed < 10.0f)
    {
        return EAnim_MovementState::Idle;
    }
    else if (MotionData.Speed < 200.0f)
    {
        return EAnim_MovementState::Walking;
    }
    else
    {
        return EAnim_MovementState::Running;
    }
}