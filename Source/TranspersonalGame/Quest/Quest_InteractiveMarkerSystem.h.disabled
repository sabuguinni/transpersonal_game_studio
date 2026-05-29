#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerBox.h"
#include "Quest_InteractiveMarkerSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_MarkerType : uint8
{
    Tutorial        UMETA(DisplayName = "Tutorial"),
    QuestGiver      UMETA(DisplayName = "Quest Giver"),
    Resource        UMETA(DisplayName = "Resource"),
    Objective       UMETA(DisplayName = "Objective"),
    Waypoint        UMETA(DisplayName = "Waypoint"),
    Danger          UMETA(DisplayName = "Danger Zone")
};

UENUM(BlueprintType)
enum class EQuest_BiomeType : uint8
{
    Pantano         UMETA(DisplayName = "Pantano"),
    Floresta        UMETA(DisplayName = "Floresta"),
    Savana          UMETA(DisplayName = "Savana"),
    Deserto         UMETA(DisplayName = "Deserto"),
    Montanha        UMETA(DisplayName = "Montanha")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MarkerData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    FString MarkerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    EQuest_MarkerType MarkerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    EQuest_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    FVector WorldLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    FLinearColor MarkerColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    float InteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    bool bIsActive;

    FQuest_MarkerData()
    {
        MarkerName = TEXT("");
        Description = TEXT("");
        MarkerType = EQuest_MarkerType::Waypoint;
        BiomeType = EQuest_BiomeType::Savana;
        WorldLocation = FVector::ZeroVector;
        MarkerColor = FLinearColor::White;
        InteractionRadius = 500.0f;
        bIsActive = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_InteractiveMarker : public AActor
{
    GENERATED_BODY()

public:
    AQuest_InteractiveMarker();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MarkerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* MarkerLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractionTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    FQuest_MarkerData MarkerData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    float PulseSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    float PulseIntensityMin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    float PulseIntensityMax;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Quest Marker")
    void SetMarkerData(const FQuest_MarkerData& NewMarkerData);

    UFUNCTION(BlueprintCallable, Category = "Quest Marker")
    void ActivateMarker();

    UFUNCTION(BlueprintCallable, Category = "Quest Marker")
    void DeactivateMarker();

    UFUNCTION(BlueprintCallable, Category = "Quest Marker")
    void SetMarkerColor(const FLinearColor& NewColor);

    UFUNCTION(BlueprintCallable, Category = "Quest Marker")
    FQuest_MarkerData GetMarkerData() const { return MarkerData; }

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Marker")
    void OnPlayerEnterMarker();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Marker")
    void OnPlayerExitMarker();

protected:
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                              UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, 
                              bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                            UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

private:
    float PulseTimer;
    bool bPlayerInRange;
    
    void UpdateMarkerVisuals();
    void UpdatePulseEffect(float DeltaTime);
    FLinearColor GetBiomeColor(EQuest_BiomeType BiomeType) const;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_InteractiveMarkerManager : public UObject
{
    GENERATED_BODY()

public:
    UQuest_InteractiveMarkerManager();

    UFUNCTION(BlueprintCallable, Category = "Quest Marker Manager")
    static AQuest_InteractiveMarker* CreateMarker(UWorld* World, const FQuest_MarkerData& MarkerData);

    UFUNCTION(BlueprintCallable, Category = "Quest Marker Manager")
    static TArray<AQuest_InteractiveMarker*> CreateBiomeMarkers(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Quest Marker Manager")
    static TArray<AQuest_InteractiveMarker*> CreateTutorialMarkers(UWorld* World, const FVector& PlayerStartLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest Marker Manager")
    static void DestroyAllMarkers(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Quest Marker Manager")
    static TArray<AQuest_InteractiveMarker*> GetMarkersInBiome(UWorld* World, EQuest_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Quest Marker Manager")
    static AQuest_InteractiveMarker* GetNearestMarker(UWorld* World, const FVector& Location, float MaxDistance = 10000.0f);

protected:
    static FVector GetBiomeCenter(EQuest_BiomeType BiomeType);
    static FQuest_MarkerData CreateBiomeMarkerData(EQuest_BiomeType BiomeType);
    static TArray<FQuest_MarkerData> CreateTutorialMarkerData(const FVector& CenterLocation);
};