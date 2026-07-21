#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "../SharedTypes.h"
#include "Quest_ResourceGatheringSystem.h"
#include "Quest_TradingSystem.generated.h"

// Trading item data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_TradeItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade Item")
    EQuest_ResourceType ItemType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade Item")
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade Item")
    int32 Value;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade Item")
    FString ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade Item")
    FString Description;

    FQuest_TradeItem()
    {
        ItemType = EQuest_ResourceType::Stone;
        Quantity = 1;
        Value = 10;
        ItemName = TEXT("Unknown Item");
        Description = TEXT("No description available");
    }
};

// Trade offer structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_TradeOffer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade Offer")
    TArray<FQuest_TradeItem> RequiredItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade Offer")
    TArray<FQuest_TradeItem> OfferedItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade Offer")
    FString OfferName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade Offer")
    bool bIsAvailable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade Offer")
    int32 MaxTrades;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade Offer")
    int32 CurrentTrades;

    FQuest_TradeOffer()
    {
        OfferName = TEXT("Basic Trade");
        bIsAvailable = true;
        MaxTrades = -1; // Unlimited
        CurrentTrades = 0;
    }
};

// NPC Trader types
UENUM(BlueprintType)
enum class EQuest_TraderType : uint8
{
    ToolTrader      UMETA(DisplayName = "Tool Trader"),
    FoodTrader      UMETA(DisplayName = "Food Trader"),
    MaterialTrader  UMETA(DisplayName = "Material Trader"),
    WeaponTrader    UMETA(DisplayName = "Weapon Trader"),
    GeneralTrader   UMETA(DisplayName = "General Trader")
};

// NPC Trader actor
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_NPCTrader : public APawn
{
    GENERATED_BODY()

public:
    AQuest_NPCTrader();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trader")
    EQuest_TraderType TraderType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trader")
    FString TraderName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trader")
    TArray<FQuest_TradeOffer> AvailableTrades;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trader")
    TArray<FQuest_TradeItem> Inventory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trader")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trader")
    bool bIsActive;

public:
    UFUNCTION(BlueprintCallable, Category = "Trader")
    bool CanTrade(const FQuest_TradeOffer& Offer) const;

    UFUNCTION(BlueprintCallable, Category = "Trader")
    bool ExecuteTrade(const FQuest_TradeOffer& Offer, class UQuest_TradingSystem* TradingSystem);

    UFUNCTION(BlueprintCallable, Category = "Trader")
    TArray<FQuest_TradeOffer> GetAvailableTrades() const;

    UFUNCTION(BlueprintCallable, Category = "Trader")
    void AddTradeOffer(const FQuest_TradeOffer& NewOffer);

    UFUNCTION(BlueprintCallable, Category = "Trader")
    void RemoveTradeOffer(const FString& OfferName);

    UFUNCTION(BlueprintCallable, Category = "Trader")
    void RestockInventory();

    UFUNCTION(BlueprintImplementableEvent, Category = "Trader")
    void OnTradeCompleted(const FQuest_TradeOffer& CompletedTrade);

    UFUNCTION(BlueprintImplementableEvent, Category = "Trader")
    void OnPlayerApproached();

private:
    void InitializeDefaultTrades();
};

// Trading system component
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_TradingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_TradingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trading")
    TArray<FQuest_TradeItem> PlayerInventory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trading")
    TArray<AQuest_NPCTrader*> NearbyTraders;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trading")
    AQuest_NPCTrader* CurrentTrader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trading")
    float TraderDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trading")
    int32 PlayerCurrency;

public:
    UFUNCTION(BlueprintCallable, Category = "Trading")
    bool AddItemToInventory(EQuest_ResourceType ItemType, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Trading")
    bool RemoveItemFromInventory(EQuest_ResourceType ItemType, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Trading")
    int32 GetItemQuantity(EQuest_ResourceType ItemType) const;

    UFUNCTION(BlueprintCallable, Category = "Trading")
    bool HasRequiredItems(const TArray<FQuest_TradeItem>& RequiredItems) const;

    UFUNCTION(BlueprintCallable, Category = "Trading")
    bool ExecuteTrade(AQuest_NPCTrader* Trader, const FQuest_TradeOffer& Offer);

    UFUNCTION(BlueprintCallable, Category = "Trading")
    TArray<AQuest_NPCTrader*> FindNearbyTraders(float SearchRadius = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Trading")
    bool StartTradingWithNPC(AQuest_NPCTrader* Trader);

    UFUNCTION(BlueprintCallable, Category = "Trading")
    void EndTrading();

    UFUNCTION(BlueprintCallable, Category = "Trading")
    TArray<FQuest_TradeOffer> GetAvailableTradesFromCurrentTrader() const;

    UFUNCTION(BlueprintCallable, Category = "Trading")
    void AddCurrency(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Trading")
    bool SpendCurrency(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Trading")
    int32 GetCurrency() const { return PlayerCurrency; }

    UFUNCTION(BlueprintImplementableEvent, Category = "Trading")
    void OnTradeCompleted(const FQuest_TradeOffer& CompletedTrade);

    UFUNCTION(BlueprintImplementableEvent, Category = "Trading")
    void OnInventoryUpdated();

    UFUNCTION(BlueprintImplementableEvent, Category = "Trading")
    void OnTradingStarted(AQuest_NPCTrader* Trader);

    UFUNCTION(BlueprintImplementableEvent, Category = "Trading")
    void OnTradingEnded();

private:
    void ScanForTraders();
    FQuest_TradeItem* FindInventoryItem(EQuest_ResourceType ItemType);
};