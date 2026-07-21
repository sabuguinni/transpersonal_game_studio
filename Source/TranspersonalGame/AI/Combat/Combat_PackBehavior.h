#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Combat_PackBehavior.generated.h"

UENUM(BlueprintType)
enum class ECombat_PackRole : uint8
{
    Alpha,      // Pack leader, makes decisions
    Beta,       // Second in command, flanks
    Hunter,     // Primary attacker
    Scout,      // Reconnaissance and early warning
    Support     // Backup and distraction
};

UENUM(BlueprintType)
enum class ECombat_PackFormation : uint8
{
    Scattered,  // Spread out patrol
    Line,       // Linear formation
    Pincer,     // Flanking from both sides
    Circle,     // Surround target
    Ambush      // Hidden positions
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PackMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    AActor* MemberActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    ECombat_PackRole Role = ECombat_PackRole::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    FVector AssignedPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsAlive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float DistanceFromAlpha = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float LastCommunicationTime = 0.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TArray<FCombat_PackMember> Members;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    ECombat_PackFormation CurrentFormation = ECombat_PackFormation::Scattered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    AActor* PrimaryTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float PackCohesion = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bInCombat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float LastFormationChangeTime = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_PackBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_PackBehavior();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    FCombat_PackData PackData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float MaxPackDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float FormationRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float CommunicationRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float FormationChangeDelay = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    bool bIsAlpha = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    ECombat_PackRole MyRole = ECombat_PackRole::Hunter;

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void InitializePack(TArray<AActor*> PackMembers);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void AddPackMember(AActor* NewMember, ECombat_PackRole Role);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void RemovePackMember(AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void SetFormation(ECombat_PackFormation NewFormation);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void SetPrimaryTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    FVector GetAssignedPosition();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void CommunicateWithPack(const FString& Message);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    bool IsPackIntact();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    AActor* GetAlphaMember();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    int32 GetLivingMemberCount();

private:
    void UpdatePackCohesion();
    void UpdateFormationPositions();
    void CheckPackIntegrity();
    FVector CalculateFormationPosition(ECombat_PackRole Role, int32 MemberIndex);
    void BroadcastToPackMembers(const FString& Message);
    
    float LastCohesionUpdate = 0.0f;
    TArray<AActor*> NearbyPackMembers;
};