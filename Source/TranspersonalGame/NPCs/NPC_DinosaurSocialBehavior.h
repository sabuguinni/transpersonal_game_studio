#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "NPC_DinosaurSocialBehavior.generated.h"

UENUM(BlueprintType)
enum class ENPC_SocialState : uint8
{
    Solitary        UMETA(DisplayName = "Solitary"),
    Seeking         UMETA(DisplayName = "Seeking Pack"),
    InPack          UMETA(DisplayName = "In Pack"),
    Mating          UMETA(DisplayName = "Mating"),
    Territorial     UMETA(DisplayName = "Territorial"),
    Fleeing         UMETA(DisplayName = "Fleeing")
};

UENUM(BlueprintType)
enum class ENPC_PackRole : uint8
{
    Alpha           UMETA(DisplayName = "Alpha"),
    Beta            UMETA(DisplayName = "Beta"),
    Scout           UMETA(DisplayName = "Scout"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Guardian        UMETA(DisplayName = "Guardian"),
    Juvenile        UMETA(DisplayName = "Juvenile")
};

USTRUCT(BlueprintType)
struct FNPC_SocialMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    AActor* Actor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    float Relationship;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    float LastInteractionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    bool bIsPackMember;

    FNPC_SocialMemory()
    {
        Actor = nullptr;
        Relationship = 0.0f;
        LastInteractionTime = 0.0f;
        bIsPackMember = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurSocialBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurSocialBehavior();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Social State Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Behavior")
    ENPC_SocialState CurrentSocialState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Behavior")
    ENPC_PackRole PackRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Behavior")
    float SocialRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Behavior")
    float PackCohesionStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Behavior")
    int32 MaxPackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Behavior")
    float TerritorialRadius;

    // Pack Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Management")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Management")
    AActor* PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Management")
    FVector PackCenter;

    // Social Memory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    TArray<FNPC_SocialMemory> SocialMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    float MemoryDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    float MaxMemoryDuration;

    // Communication
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float CommunicationRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float LastCommunicationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float CommunicationCooldown;

    // Mating Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mating")
    bool bIsMatingSeason;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mating")
    float MatingDesire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mating")
    AActor* MatingTarget;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Social Behavior")
    void UpdateSocialState();

    UFUNCTION(BlueprintCallable, Category = "Social Behavior")
    void ScanForNearbyDinosaurs();

    UFUNCTION(BlueprintCallable, Category = "Pack Management")
    void JoinPack(AActor* NewPackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack Management")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack Management")
    void EstablishPackLeadership();

    UFUNCTION(BlueprintCallable, Category = "Pack Management")
    FVector CalculatePackCenter();

    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    void AddSocialMemory(AActor* Actor, float RelationshipValue);

    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    FNPC_SocialMemory* GetSocialMemory(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    void UpdateSocialMemories(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void CommunicateWithPack(const FString& Message);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void RespondToPackCommunication(AActor* Sender, const FString& Message);

    UFUNCTION(BlueprintCallable, Category = "Territorial")
    bool IsInTerritory(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Territorial")
    void DefendTerritory(AActor* Intruder);

    UFUNCTION(BlueprintCallable, Category = "Mating")
    void InitiateMatingBehavior();

    UFUNCTION(BlueprintCallable, Category = "Mating")
    AActor* FindMatingPartner();

private:
    float TimeSinceLastSocialUpdate;
    float SocialUpdateInterval;
    
    void ProcessSocialInteractions();
    void UpdatePackBehavior();
    void HandleTerritorialBehavior();
    void ProcessMatingBehavior();
    float CalculateRelationship(AActor* OtherActor);
    bool CanFormPack(AActor* OtherActor);
    void SendPackSignal(const FString& SignalType);
};