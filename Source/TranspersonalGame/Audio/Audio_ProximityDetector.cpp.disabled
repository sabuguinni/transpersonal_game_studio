#include "Audio_ProximityDetector.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

UAudio_ProximityDetector::UAudio_ProximityDetector()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Atualizar a cada 0.5 segundos
    
    DetectionRadius = 10000.0f; // 100 metros em UU
    UpdateInterval = 0.5f;
    LastUpdateTime = 0.0f;
    
    // Distâncias de ameaça
    CriticalDistance = 500.0f;  // 5 metros
    HighDistance = 2000.0f;     // 20 metros
    MediumDistance = 5000.0f;   // 50 metros
    
    // Tags padrão para dinossauros
    TargetTags.Add(TEXT("TRex"));
    TargetTags.Add(TEXT("Raptor"));
    TargetTags.Add(TEXT("Brachiosaurus"));
    TargetTags.Add(TEXT("Dinosaur"));
    TargetTags.Add(TEXT("Predator"));
}

void UAudio_ProximityDetector::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_ProximityDetector: Detector de proximidade iniciado"));
    UE_LOG(LogTemp, Warning, TEXT("Audio_ProximityDetector: Raio de detecção: %f UU"), DetectionRadius);
    
    // Primeira detecção imediata
    UpdateProximityDetection();
}

void UAudio_ProximityDetector::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= UpdateInterval)
    {
        UpdateProximityDetection();
        LastUpdateTime = 0.0f;
    }
}

void UAudio_ProximityDetector::SetDetectionRadius(float NewRadius)
{
    DetectionRadius = FMath::Max(NewRadius, 100.0f); // Mínimo 1 metro
    UE_LOG(LogTemp, Warning, TEXT("Audio_ProximityDetector: Raio de detecção alterado para %f"), DetectionRadius);
}

void UAudio_ProximityDetector::AddTargetTag(const FString& Tag)
{
    if (!TargetTags.Contains(Tag))
    {
        TargetTags.Add(Tag);
        UE_LOG(LogTemp, Warning, TEXT("Audio_ProximityDetector: Tag adicionada: %s"), *Tag);
    }
}

void UAudio_ProximityDetector::RemoveTargetTag(const FString& Tag)
{
    if (TargetTags.Contains(Tag))
    {
        TargetTags.Remove(Tag);
        UE_LOG(LogTemp, Warning, TEXT("Audio_ProximityDetector: Tag removida: %s"), *Tag);
    }
}

void UAudio_ProximityDetector::UpdateProximityDetection()
{
    if (!GetWorld() || !GetOwner())
    {
        return;
    }
    
    // Limpar ameaças anteriores
    NearbyThreats.Empty();
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Obter todos os actores no mundo
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor || Actor == GetOwner())
        {
            continue;
        }
        
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        
        if (Distance <= DetectionRadius)
        {
            ProcessDetectedActor(Actor);
        }
    }
    
    // Ordenar ameaças por distância (mais próximas primeiro)
    NearbyThreats.Sort([](const FAudio_ProximityData& A, const FAudio_ProximityData& B) {
        return A.Distance < B.Distance;
    });
    
    // Disparar eventos para ameaças críticas
    for (const FAudio_ProximityData& Threat : NearbyThreats)
    {
        if (Threat.ThreatLevel >= EAudio_ThreatLevel::High)
        {
            OnProximityAlert.Broadcast(Threat);
        }
    }
    
    if (NearbyThreats.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_ProximityDetector: %d ameaças detectadas"), NearbyThreats.Num());
    }
}

EAudio_ThreatLevel UAudio_ProximityDetector::CalculateThreatLevel(float Distance) const
{
    if (Distance <= CriticalDistance)
    {
        return EAudio_ThreatLevel::Critical;
    }
    else if (Distance <= HighDistance)
    {
        return EAudio_ThreatLevel::High;
    }
    else if (Distance <= MediumDistance)
    {
        return EAudio_ThreatLevel::Medium;
    }
    else
    {
        return EAudio_ThreatLevel::Low;
    }
}

float UAudio_ProximityDetector::CalculateAudioIntensity(float Distance, EAudio_ThreatLevel ThreatLevel) const
{
    // Intensidade baseada na distância (inversa)
    float DistanceIntensity = 1.0f - (Distance / DetectionRadius);
    
    // Multiplicador baseado no nível de ameaça
    float ThreatMultiplier = 1.0f;
    switch (ThreatLevel)
    {
        case EAudio_ThreatLevel::Critical:
            ThreatMultiplier = 2.0f;
            break;
        case EAudio_ThreatLevel::High:
            ThreatMultiplier = 1.5f;
            break;
        case EAudio_ThreatLevel::Medium:
            ThreatMultiplier = 1.2f;
            break;
        case EAudio_ThreatLevel::Low:
            ThreatMultiplier = 0.8f;
            break;
        default:
            ThreatMultiplier = 0.5f;
            break;
    }
    
    return FMath::Clamp(DistanceIntensity * ThreatMultiplier, 0.0f, 2.0f);
}

void UAudio_ProximityDetector::ProcessDetectedActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    FString ActorName = Actor->GetName();
    FString ActorLabel = Actor->GetActorLabel();
    
    // Verificar se o actor tem uma das tags alvo
    bool bIsTarget = false;
    for (const FString& Tag : TargetTags)
    {
        if (ActorName.Contains(Tag) || ActorLabel.Contains(Tag))
        {
            bIsTarget = true;
            break;
        }
    }
    
    // Também considerar pawns como potenciais ameaças
    if (!bIsTarget && Actor->IsA<APawn>())
    {
        bIsTarget = true;
    }
    
    if (bIsTarget)
    {
        FVector OwnerLocation = GetOwner()->GetActorLocation();
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        
        FAudio_ProximityData ProximityData;
        ProximityData.Distance = Distance;
        ProximityData.ThreatLevel = CalculateThreatLevel(Distance);
        ProximityData.TargetName = ActorLabel.IsEmpty() ? ActorName : ActorLabel;
        ProximityData.TargetLocation = Actor->GetActorLocation();
        ProximityData.AudioIntensity = CalculateAudioIntensity(Distance, ProximityData.ThreatLevel);
        
        NearbyThreats.Add(ProximityData);
        
        UE_LOG(LogTemp, Warning, TEXT("Audio_ProximityDetector: Ameaça detectada - %s a %f UU (Nível: %d)"), 
               *ProximityData.TargetName, Distance, (int32)ProximityData.ThreatLevel);
    }
}