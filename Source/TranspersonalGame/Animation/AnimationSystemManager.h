#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "../SharedTypes.h"
#include "AnimationSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle,
    Walking,
    Running,
    Crouching,
    Jumping,
    Falling,
    Swimming,
    Climbing,
    Combat
};

UENUM(BlueprintType)
enum class EAnim_CharacterType : uint8
{
    Player,
    TribalNPC,
    Shaman,
    Hunter,
    Gatherer,
    Child,
    Elder,
    Dinosaur_Small,
    Dinosaur_Medium,
    Dinosaur_Large
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsAccelerating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    EAnim_MovementState MovementState;

    FAnim_MotionData()
    {
        Velocity = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsAccelerating = false;
        bIsInAir = false;
        MovementState = EAnim_MovementState::Idle;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector LeftFootLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector RightFootLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator LeftFootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator RightFootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float HipOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bEnableFootIK;

    FAnim_IKData()
    {
        LeftFootLocation = FVector::ZeroVector;
        RightFootLocation = FVector::ZeroVector;
        LeftFootRotation = FRotator::ZeroRotator;
        RightFootRotation = FRotator::ZeroRotator;
        HipOffset = 0.0f;
        bEnableFootIK = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_CharacterProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EAnim_CharacterType CharacterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float RunSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float PersonalityFactor; // 0.0 = calm, 1.0 = energetic

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<UAnimMontage*> AvailableMontages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    UBlendSpace* MovementBlendSpace;

    FAnim_CharacterProfile()
    {
        CharacterType = EAnim_CharacterType::Player;
        CharacterName = TEXT("DefaultCharacter");
        WalkSpeed = 150.0f;
        RunSpeed = 400.0f;
        PersonalityFactor = 0.5f;
        MovementBlendSpace = nullptr;
    }
};

/**
 * Animation System Manager - Handles motion matching, IK, and character-specific animation
 * This is the heart of our animation system, inspired by RDR2's approach to character movement
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnimationSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UAnimationSystemManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core animation functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void RegisterCharacter(AActor* Character, EAnim_CharacterType CharacterType);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UnregisterCharacter(AActor* Character);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    FAnim_MotionData CalculateMotionData(AActor* Character);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    FAnim_IKData CalculateFootIK(AActor* Character, USkeletalMeshComponent* SkeletalMesh);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateCharacterAnimation(AActor* Character, float DeltaTime);

    // Motion matching system
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    UAnimSequence* FindBestMatchingAnimation(const FAnim_MotionData& MotionData, EAnim_CharacterType CharacterType);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BuildMotionDatabase();

    // Character personality system
    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetCharacterPersonality(AActor* Character, float PersonalityFactor);

    UFUNCTION(BlueprintCallable, Category = "Character")
    FAnim_CharacterProfile GetCharacterProfile(AActor* Character);

    // IK system
    UFUNCTION(BlueprintCallable, Category = "IK")
    FVector TraceFootPlacement(AActor* Character, FVector FootWorldLocation);

    UFUNCTION(BlueprintCallable, Category = "IK")
    void EnableFootIK(AActor* Character, bool bEnable);

    // Animation montage system
    UFUNCTION(BlueprintCallable, Category = "Montage")
    void PlayCharacterMontage(AActor* Character, UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Montage")
    void StopCharacterMontage(AActor* Character, float BlendOutTime = 0.25f);

    // Debug and visualization
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugAnimationSystem();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawMotionDebugInfo(AActor* Character);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawIKDebugInfo(AActor* Character);

protected:
    // Character registry
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    TMap<AActor*, FAnim_CharacterProfile> RegisteredCharacters;

    // Motion matching database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TMap<EAnim_CharacterType, TArray<UAnimSequence*>> MotionDatabase;

    // Animation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float IKTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float IKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableMotionMatching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bDrawDebugInfo;

private:
    // Internal helper functions
    void InitializeMotionDatabase();
    void LoadAnimationAssets();
    USkeletalMeshComponent* GetCharacterSkeletalMesh(AActor* Character);
    FVector GetCharacterVelocity(AActor* Character);
    bool IsCharacterInAir(AActor* Character);
    float CalculateMovementDirection(AActor* Character);
    EAnim_MovementState DetermineMovementState(const FAnim_MotionData& MotionData);
};